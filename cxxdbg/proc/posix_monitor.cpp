// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// process_monitor.cpp
/// Contains implementation of the process_monitor class.

#include "posix_monitor.hpp"
#include <boost/asio/post.hpp>


namespace cxxdbg::proc {


posix_monitor::posix_monitor(boost::asio::executor exec, pid_t pid):
exec_{std::move(exec)}, pid_{pid} {
    // starting monitor thread
    thrd_ = std::thread{[this] { monitor_thread_func(); }};
}


posix_monitor::~posix_monitor() {
    thrd_.join();
}


void posix_monitor::terminate() {
    ::kill(pid_, SIGKILL);
}


void posix_monitor::monitor_thread_func() {
    // waiting for process exit
    while (true) {
        if (::kill(pid_, 0) == -1) {
            // process died
            boost::asio::post(exec_, [this] {
                pid_ = 0;
                exited(-1);
            });
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds{100});
    }
}


}
