// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file exec_stub.cpp
/// execution stub implementation for running executable being debugged
/// in terminal

#include "exec_stub_client.hpp"
#include "cxxdbg/log/log_init.hpp"
#include "cxxdbg/util/command_line_parser.hpp"
#include <iostream>
#include "cxxdbg/boost_process/process.hpp"
#include <boost/program_options.hpp>

#ifdef __WIN32
#include <windows.h>
#endif

#ifndef __WIN32
#include <sys/ptrace.h>
#endif

#ifdef __linux__
#include <sys/prctl.h>
#endif // __linux__


#define CXXDBG_PR_SET_PTRACER 0x59616d61


namespace bp = boost::process;
namespace po = boost::program_options;
using namespace cxxdbg::dbg::exec_stub;



#ifdef __WIN32

/// This function performs the following:
/// 1) Launches process via CreateProcess
/// 2) Open shared memory object with specified name
/// 3) Writes current process ID to shared memory and notifies cxxdbg
void do_exec(const std::string & shm_name,
             const std::vector<std::string> & args,
             bool debug_log,
             bool nodebug,
             bool nostop) {

    assert(!args.empty() && "args should not be empty");

    if (debug_log) {
        std::cerr << "executing CreateProcess...\n";
    }

    std::ostringstream cmd_line_str;
    bool first = true;
    for (auto && arg : args) {
        if (first) {
            first = false;
        } else {
            cmd_line_str << ' ';
        }
        cmd_line_str << '\"' << arg << '\"';
    }

    auto cmd_line = cmd_line_str.str();

    DWORD flags = 0;
    if (!nodebug) {
        flags |= CREATE_SUSPENDED;
    }

    PROCESS_INFORMATION proc_info = {0, 0, 0, 0};
    STARTUPINFO sinfo = {0};
    auto res = ::CreateProcessA(NULL,
                                (char*)cmd_line.c_str(),
                                NULL,
                                NULL,
                                FALSE,
                                flags,
                                NULL,
                                NULL,
                                &sinfo,
                                &proc_info);
    if (!res) {
        throw std::runtime_error("CreateProcess failed");
    }

    if (debug_log) {
        std::cerr << "created process " << proc_info.dwProcessId << "\n";
    }

#if 0
    if (!nodebug) {
        HANDLE exe_image = nullptr;

        // eating all debug events
        while (true) {
            DEBUG_EVENT event;

            if (debug_log) {
                std::cerr << "reading next debug event...\n";
            }

            if (!::WaitForDebugEvent(&event, INFINITE)) {
                throw std::runtime_error{"WaitForDebugEevent failed"};
            }

            if (event.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
                if (debug_log) {
                    std::cerr << "received CREATE_PROCESS_DEBUG_EVENT\n";
                }

                // saving handle to image to close it after
                exe_image = event.u.CreateProcessInfo.hFile;
            }

            if (event.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
                if (debug_log) {
                    std::cerr << "received EXCEPTION_DEBUG_EVENT\n";
                }

                // First exception event indicates completion of launch.
                // We need suspend start thread here
                if (::SuspendThread(proc_info.hThread)) {
                    throw std::runtime_error{"SuspendThread failed"};
                }
            }

            // continue thread after debug event
            if (!::ContinueDebugEvent(event.dwProcessId, event.dwThreadId, DBG_CONTINUE)) {
                throw std::runtime_error{"ContinueDebugEvent failed"};
            }

            // stopping eating of debug events after receiving EXCEPTION_DEBUG_EVENT
            if (event.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
                break;
            }
        }

        // detaching from process
        if (!::DebugActiveProcessStop(proc_info.dwProcessId)) {
            throw std::runtime_error{"DebugActiveProcessStop failed"};
        }

        if (exe_image) {
            ::CloseHandle(exe_image);
        }
    }
#endif

    if (!shm_name.empty()) {
        // sending PID

        if (debug_log) {
            std::cerr << "sending child pid via '" << shm_name << "' semaphore...\n";
        }

        exec_stub_client estb_client(shm_name);
        estb_client.send_pid(static_cast<unsigned long>(proc_info.dwProcessId),
                             static_cast<unsigned long>(proc_info.dwThreadId));
    } else {
        if (debug_log) {
            std::cerr << "semaphore name is not set, not sending child pid...\n";
        }
    }

    if (debug_log) {
        std::cerr << "waiting for process...\n";
    }

    ::WaitForSingleObject(proc_info.hProcess, INFINITE);

    if (debug_log) {
        std::cerr << "process wait complete\n";
    }

    DWORD exit_code;
    ::GetExitCodeProcess(proc_info.hProcess, &exit_code);
    std::cerr << "Process exited with exit code " << exit_code << "\n";
}

#else

/// This function calls fork and performs the following:
/// For parent:
/// 1) Waits for child stopped after PTRACE_TRACEME
/// 1) Open shared memory object with specified name
/// 2) Writes current process ID to shared memory and notifies cxxdbg
/// For child:
/// 3) Executes ptrace(TRACEME)
/// 4) Calls exec
void do_exec(const std::string & shm_name,
             const std::vector<std::string> & args,
             bool debug_log,
             bool nodebug,
             bool nostop) {

    assert(!args.empty() && "args should not be empty");

    // ignoring SIGTTOU signal (tcsetpgrp sends it and blocks execution)

    if (debug_log) {
        std::cerr << "installing SIGTTOU ignore handler...\n";
    }

    {
        struct sigaction act;
        ::memset(&act, 0, sizeof(act));
        act.sa_handler = SIG_IGN;
        if (::sigaction(SIGTTOU, &act, 0) == -1) {
            throw std::runtime_error("sigaction(SIGTTOU) failed");
        }
    }

    if (debug_log) {
        std::cerr << "executing fork...\n";
    }

    pid_t pid = ::fork();
    if (pid == -1) {
        throw std::runtime_error("fork failed");
    } else if (pid == 0) {
        // child

        // put process in its own group and make it foreground
        // group in terminal (to receive Cntrl+C, etc)
        ::setpgid(0, 0);
        ::tcsetpgrp(0, ::getpid());

        // preparing child process for debugging
        if (!nodebug) {
#ifdef __linux__
            // saying that anyone can debug this process
            ::prctl(CXXDBG_PR_SET_PTRACER, static_cast<unsigned long>(-1));
#endif // __linux__

            // listening for continue notification from exec stub server
            if (!nostop && !shm_name.empty()) {
                if (debug_log) {
                    std::cerr << "waiting for resume signal from debugger\n";
                }

                exec_stub_client estb_client(shm_name);
                estb_client.wait_continue();

                if (debug_log) {
                    std::cerr << "received resume signal from debugger, continue execution\n";
                }
            }
        }

        // executing command

        std::vector<char*> pargs;
        for (auto it = args.begin(), end = args.end(); it != end; ++it) {
            pargs.push_back(const_cast<char*>(it->c_str()));
        }

        pargs.push_back(nullptr);
        execv(args.front().c_str(), &pargs[0]);

        std::cerr << "Exec failed\n";
        ::exit(2);

    } else {
        // parent

        if (debug_log) {
            std::cerr << "fork successed, child PID = " << pid << "\n";
        }

        if (debug_log) {
            std::cerr << "waiting for child...\n";
        }

        if (!shm_name.empty()) {
            // sending PID

            if (debug_log) {
                std::cerr << "sending child pid via '" << shm_name << "' semaphore...\n";
            }

            exec_stub_client estb_client(shm_name);
            estb_client.send_pid(static_cast<unsigned long>(pid), 0);
        } else {
            if (debug_log) {
                std::cerr << "semaphore name is not set, not sending child pid...\n";
            }
        }

        if (debug_log) {
            std::cerr << "waiting for child exit...\n";
        }

        // waiting for child exit
        while (true) {

            if (debug_log) {
                std::cerr << "executing waitpid...\n";
            }

            int status;
            int res = ::waitpid(-1, &status, 0);
            if (res == -1) {
                if (errno == EINTR) {
                    // was interrupted by a signal, continue

                    if (debug_log) {
                        std::cerr << "waitpid was interrupted by a signal, continue...\n";
                    }

                    continue;
                }
                throw std::runtime_error("::waitpid failed");
            }

            if (WIFEXITED(status)) {
                std::cerr << "Process exited with exit code " << WEXITSTATUS(status) << "\n";
                break;
            } else if (WIFSIGNALED(status)) {
                std::cerr << "Process exited with signal " << WTERMSIG(status) << "\n";
                break;
            } else {
                throw std::runtime_error("waitpid returned unknown child status");
            }
        }
    }
}

#endif


int main(int argc, char * argv[]) {

    try {
        // parsing --args option
        std::vector<std::string> args;
        std::vector<std::string> regular_opts;

        bool args_parsed = cxxdbg::util::parse_args_command_line(argv,
                                                               argv + argc,
                                                               regular_opts,
                                                               args);

        po::options_description opt_desc;
        opt_desc.add_options()
                ("shm", po::value<std::string>(), "shared memory object name")
                ("debug-log", "print debug log")
                ("nodebug", "launch child without debugging")
                ("nostop", "don't stop child process after exec")
                ("help", "produce help message and exit");

        opt_desc.add(cxxdbg::log::log_options());

        po::variables_map var_map;
        po::store(po::command_line_parser(regular_opts).options(opt_desc).run(), var_map);

        if (var_map.count("help") > 0) {
            opt_desc.print(std::cout);
            return 1;
        }

        cxxdbg::log::init(var_map);

        if (!args_parsed) {
            throw std::runtime_error("--args option is not specified");
        }

        if (args.empty()) {
            throw std::runtime_error("program name should follow --args");
        }

        std::string shm = var_map.count("shm") > 0 ?
                          var_map["shm"].as<std::string>() :
                          std::string();

        do_exec(shm,
                args,
                var_map.count("debug-log") > 0,
                var_map.count("nodebug") > 0,
                var_map.count("nostop") > 0);

        return 0;
    }
    catch(std::exception & ex) {
        std::cerr << "ERROR: " << ex.what() << "\n";
        return 2;
    }
}

