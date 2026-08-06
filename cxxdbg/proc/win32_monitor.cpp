// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file win32_monitor.cpp
/// Contains implementation of the win32_monitor class.

#include "win32_monitor.hpp"
#include <boost/asio/post.hpp>
#include <sstream>
#include <thread>


namespace cxxdbg::proc {


win32_monitor::win32_monitor(boost::asio::executor exec, pid_t pid):
exec_{std::move(exec)} {
    // opening process
    hprocess_ = ::OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, static_cast<DWORD>(pid));
    if (!hprocess_) {
        std::ostringstream msg;
        msg << "OpenProcess failed with error code " << ::GetLastError();
        throw std::runtime_error{msg.str()};
    }

    // starting monitor thread
    thrd_ = std::thread{[this] { monitor_thread_func(); }};
}


win32_monitor::~win32_monitor() {
    thrd_.join();
}


void win32_monitor::terminate() {
    ::TerminateProcess(hprocess_, 127);
}


void win32_monitor::monitor_thread_func() {
    // waiting for process exit
    ::WaitForSingleObject(hprocess_, INFINITE);

    // process exited, executing cleanup code in main thread
    boost::asio::post(exec_, [this] {
        // getting process exit code
        DWORD exit_code;
        ::GetExitCodeProcess(hprocess_, &exit_code);

        // closing process handle
        ::CloseHandle(hprocess_);
        hprocess_ = nullptr;

        // notifying clients
        exited(static_cast<int>(exit_code));;
    });
}


}