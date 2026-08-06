// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file posix_monitor.hpp
/// Contains definition of the posix_monitor class.

#pragma once

#include "monitor.hpp"
#include <boost/asio/executor.hpp>
#include <thread>


namespace cxxdbg::proc {


/// Process monitor for POSIX platforms. Creates thread and monitors process
/// using kill function
class posix_monitor: public monitor {
public:
    /// Constructs posix monitor with ASIO executor for posting exit event
    /// and ID of process
    posix_monitor(boost::asio::executor exec, pid_t pid);

    /// Waits until process dies and destroys monitor
    ~posix_monitor() override;

    /// Starts process termination using kill function
    void terminate() override;

private:
    /// Monitor thread function
    void monitor_thread_func();

    boost::asio::executor exec_;            ///< Main thread executor
    pid_t pid_;                             ///< Process ID
    std::thread thrd_;                      ///< Monitor thread
};


}
