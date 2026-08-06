// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file bp_child_monitor.hpp
/// Contains definition of the bp_child_monitor class.

#pragma once

#include "child_io_monitor.hpp"
#include "log.hpp"

#include <boost/asio/executor.hpp>
#include "cxxdbg/boost_process/process.hpp"
#include <thread>


namespace cxxdbg::proc {


/// Monitor that uses boost process for monitoring and controlling child process
class bp_child_monitor: public monitor {
public:
    /// Constructs child monitor with specified executor for posting events and
    /// rvalue reference to boost process child object
    bp_child_monitor(boost::asio::executor event_exec, boost::process::child && chld);

    /// Destroys child monitor. Checks that child process exited
    ~bp_child_monitor() override;

    /// Starts process termination
    void terminate() override;

private:
    /// Process monitor thread function
    void thread_func();

    boost::asio::executor event_exec_;  ///< ASIO executor for posting events
    boost::process::child child_;       ///< Child process
    std::thread thrd_;                  ///< Thread for monitorring process
};


}
