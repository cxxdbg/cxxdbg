// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file win32_monitor.hpp
/// Contains definition of the win32_monitor class.

#pragma once

#include "monitor.hpp"
#include <boost/asio/executor.hpp>
#include <thread>


namespace cxxdbg::proc {


/// Win32 process monitor. Uses OpenProcess to open process and monitor its status.
class win32_monitor: public monitor {
public:
    /// Constructs monitor with specified executor for posting events and process ID
    win32_monitor(boost::asio::executor exec, pid_t pid);

    /// Waits until process dies and destroys monitor
    ~win32_monitor() override;

    /// Starts process termination
    void terminate() override;

private:
    /// Monitor thread function
    void monitor_thread_func();

    boost::asio::executor exec_;        ///< ASIO executor for posting events
    void * hprocess_;                   ///< Process handle
    std::thread thrd_;                  ///< Monitor thread
};


}
