// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file client_term_exec_stub_launcher.cpp
/// Contains implementation of the client_term_exec_stub_launcher class.

#include "client_term_exec_stub_launcher.hpp"
#include "server_impl.hpp"
#include "cxxdbg/exec_stub/exec_stub_server.hpp"
#include "cxxdbg/proc/system_monitor.hpp"
#include <boost/dll.hpp>
#include "cxxdbg/boost_process/process.hpp"

namespace bp = boost::process;


namespace cxxdbg::dbg::dapsrv {


void client_term_exec_stub_launcher::launch_async(const proc::launch_parameters & pars, const handler & comp_handler) {
    // creating exec_stub server for exchanging data with exec_stub executable
    auto exec_stub_srv = std::make_shared<exec_stub::exec_stub_server>();

    // getting path to cxxdbg-exec-stub executable
    auto exec_stub_path = std::filesystem::path{boost::dll::program_location().string()};
    exec_stub_path.remove_filename();

#ifdef _WIN32
    auto exec_stub_name = "cxxdbg-exec-stub.exe";
#else
    auto exec_stub_name = "cxxdbg-exec-stub";
#endif
    exec_stub_path /= exec_stub_name;

    auto shm_name = exec_stub_srv->shared_mem_name();

    // building cxxdbg-exec-stub args
    std::vector<std::wstring> args;
    args.push_back(exec_stub_path.wstring());
    args.push_back(L"--nodebug");
    args.push_back(std::wstring(L"--shm=") + std::wstring{shm_name.begin(), shm_name.end()});
    args.push_back(L"--args");
    if (pars.cmd.empty()) {
        assert(!pars.exe.empty() && "both command line and executable are empty");
        args.push_back(pars.exe.wstring());
        std::copy(pars.args.begin(), pars.args.end(), std::back_inserter(args));
    } else {
        assert("support of command line in cxxdbg-exec-stub terminal launcher is not implemented");
    }

    // launching process via launcher
    s_impl_.send_run_in_terminal(pars.work_dir, args, true, {}, pars.env,
    [this, comp_handler, exec_stub_srv](long, long, const std::string & err) {
        if (!err.empty()) {
            // error launching
            std::ostringstream msg;
            msg << "error launching cxxdbg-exec-stub: ";
            if (!err.empty()) {
                msg << err;
            } else {
                msg << "unknown error";
            }
            comp_handler({}, msg.str());
        } else {
            // exec stub was launched

            unsigned long pid = 0;
            unsigned long tid = 0;

            // waiting for PID
            for (unsigned int count = 0; count < 20; ++count) {
                std::chrono::nanoseconds dur =
                        std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(500));
                
                auto [xpid, xtid] = exec_stub_srv->wait_pid(dur);
                pid = xpid;
                tid = xtid;

                if (pid != 0) {
                    break;
                }
            }

            if (pid == 0) {
                // can't get PID for launched process
                comp_handler({}, "waiting for executable PID timed out");
                return;
            }

            // creating system process monitor for PID
            auto child_monitor = std::make_unique<proc::system_monitor>(s_impl_.get_executor(), pid);
            comp_handler(std::move(child_monitor), {});
        }
    });
}


}
