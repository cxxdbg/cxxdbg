// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file bp_child_monitor.cpp
/// Contains implementation of the bp_child_monitor class.

#include "bp_child_monitor.hpp"
#include <boost/asio/post.hpp>
#include <chrono>
#include <memory>


namespace cxxdbg::proc {


bp_child_monitor::bp_child_monitor(boost::asio::executor event_exec, boost::process::child && chld):
event_exec_{std::move(event_exec)},
child_{std::move(chld)} {
    // starting monitor thread
    thrd_ = std::thread{[this] { thread_func(); }};
}


bp_child_monitor::~bp_child_monitor() {
    assert(!child_.running() && "process is still running");
    assert(child_.joinable() && "process is not joinable");
    child_.join();
    thrd_.join();
}


void bp_child_monitor::terminate() {
    child_.terminate();
}


void bp_child_monitor::thread_func() {
    // waiting for process termination
    child_.wait();
    assert(!child_.running() && "process is still running");

    // sending exit signal
    int exit_code = child_.exit_code();
    boost::asio::post(event_exec_, [this, exit_code] {
        exited(exit_code);
    });
}


}
