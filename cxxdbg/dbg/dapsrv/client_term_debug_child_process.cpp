// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file client_term_debug_child_process.cpp
/// Contains implementation of the client_term_debug_child_process class.

#include "client_term_debug_child_process.hpp"
#include "log.hpp"
#include "cxxdbg/boost_process/process.hpp"
#include <boost/dll.hpp>


namespace fs = std::filesystem;


namespace cxxdbg::dbg::dapsrv {


void client_term_debug_child_process::prepare_launch(const path_t & exe_path,
                                                     const path_t & work_dir,
                                                     const std::vector<std::wstring> & args,
                                                     const std::map<std::wstring, std::wstring> & env,
                                                     const json_t & json_parameters,
                                                     const source_breakpoints_map & source_breakpoints,
                                                     const function_breakpoints_vector & function_breakpoints,
                                                     const instruction_breakpoints_vector & instruction_breakpoints,
                                                     const std::vector<std::wstring> & exception_breakpoint_filters) {

    // saving common launch parameters
    exe_path_ = exe_path;
    work_dir_ = work_dir;
    args_ = args;
    env_ = env;

    // delegating all other work to base class
    debug_child_process::prepare_launch(exe_path,
                                        work_dir,
                                        args,
                                        env,
                                        json_parameters,
                                        source_breakpoints,
                                        function_breakpoints,
                                        instruction_breakpoints,
                                        exception_breakpoint_filters);
}


void client_term_debug_child_process::process_configuration_done(const std::function<void (const std::string &)> & comp_handler) {
    if (was_attached()) {
        // delegating work to base class
        debug_child_process::process_configuration_done(comp_handler);
    } else {
        // launching new process
        std::ostringstream args_str;
        args_str << "[";
        bool first = true;
        for (auto && arg : args_) {
            args_str << "'" << std::string{arg.begin(), arg.end()} << "'";
            if (first) {
                args_str << ", ";
            }
            first = false;
        }
        args_str << "]";

        // creating exec_stub server for exchanging data with exec_stub executable and debuggee
        exec_stub_srv_ = std::make_unique<exec_stub::exec_stub_server>();

        // getting path to cxxdbg-exec-stub executable
        std::filesystem::path exec_stub_path{boost::dll::program_location().string()};
        exec_stub_path.remove_filename();
        exec_stub_path /= "cxxdbg-exec-stub";

        // building exec_stub args
        auto shm_name = exec_stub_srv_->shared_mem_name();
        std::vector<std::wstring> args;
        args.push_back(std::wstring(L"--shm=") + std::wstring{shm_name.begin(), shm_name.end()});
        args.push_back(L"--args");
        args.push_back(exe_path_.wstring());
        std::copy(args_.begin(), args_.end(), std::back_inserter(args));

        // launching process via launcher
        proc_launcher_->launch(exec_stub_path, work_dir_, args, env_,
        [this, comp_handler](long, const std::string & err) {
            try {
                if (!err.empty()) {
                    // error launching
                    std::ostringstream msg;
                    msg << "error launching debug: ";
                    if (!err.empty()) {
                        msg << err;
                    } else {
                        msg << "unknown error";
                    }
                    comp_handler(msg.str());
                } else {
                    // exec stub was launched

                    CXXDBG_DAP_LOG_TRACE << "waiting for PID from cxxdbg-exec-stub...";

                    // waiting for PID
                    unsigned long pid = 0;
                    unsigned long tid = 0;

                    for (unsigned int count = 0; count < 20; ++count) {
                        // waiting for PID
                        std::chrono::nanoseconds dur =
                                std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(500));
                        std::tie(pid, tid) = exec_stub_srv_->wait_pid(dur);

                        if (pid != 0) {
                            break;
                        }
                    }

                    if (pid == 0) {
                        // can't get PID for launched process
                        CXXDBG_DAP_LOG_ERROR << "can't receive PID from exec_stub, timed out";
                        comp_handler("waiting for executable PID timed out");
                        return;
                    }

// TODO: check number of stops required for non linux platforms
#if defined(__APPLE__)
                    set_num_skip_init_stops(2);
#elif defined(__WIN32)
                    set_num_skip_init_stops(1);
#else
                    set_num_skip_init_stops(2);
#endif

                    CXXDBG_DAP_LOG_DEBUG << "attaching to process " << pid << "...";

                    // attaching to process with PID received from exec_stub
                    targ().attach(pid);

                    // sending continue notification to debuggee process via exec stub server
                    exec_stub_srv_->notify_continue();

                    CXXDBG_DAP_LOG_DEBUG << "attached to process " << pid;

#ifdef __WIN32
                    // resuming suspended main thread on Windows
                    CXXDBG_DAP_LOG_DEBUG << "resuming thread " << tid;
                    
                    auto hproc = ::OpenProcess(PROCESS_SUSPEND_RESUME , FALSE, pid);
                    if (!hproc) {
                        std::ostringstream msg;
                        msg << "OpenProcess failed for process " << pid;
                        CXXDBG_DAP_LOG_ERROR << msg.str();
                        comp_handler(msg.str());
                        return;
                    }

                    auto hthread = ::OpenThread(THREAD_SUSPEND_RESUME, FALSE, tid);
                    if (!hthread) {
                        std::ostringstream msg;
                        msg << "OpenThread failed for thread " << tid;
                        CXXDBG_DAP_LOG_ERROR << msg.str();
                        ::CloseHandle(hproc);
                        comp_handler(msg.str());
                        return;
                    }

                    auto res = ::ResumeThread(hthread);
                    ::CloseHandle(hthread);
                    ::CloseHandle(hproc);

                    if (res == (DWORD)-1) {
                        std::ostringstream msg;
                        msg << "ResumeThread failed for thread " << tid;
                        CXXDBG_DAP_LOG_ERROR << msg.str();
                        comp_handler(msg.str());
                        return;
                    }
#endif

                    comp_handler({});
                }
            }
            catch (std::exception & err) {
                std::ostringstream msg;
                msg << "error attaching to process: " << err.what();
                CXXDBG_DAP_LOG_ERROR << msg.str();
                comp_handler(msg.str());
            }
        });
    }
}


}
