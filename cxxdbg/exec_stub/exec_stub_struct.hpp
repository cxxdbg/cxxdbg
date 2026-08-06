// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file exec_stub_struct.hpp
/// Contains definition of the exec_stub_struct class.

#pragma once

#include "boost/date_time/posix_time/posix_time_types.hpp"
#include <boost/date_time/time_duration.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>


namespace cxxdbg::dbg::exec_stub {


/// \class exec_stub_struct
/// Represents data structure for exchanging data between cxxdbg, exec_stub, and debuggee
class exec_stub_struct {
public:
    /// Constructs exec_stub_struct with zero PID
    exec_stub_struct();

    /// Destroys exec_stub_struct
    ~exec_stub_struct();

    /// Returns ID of cxxdbg-exec-stub process
    unsigned long pid();

    /// Sets ID of cxxdbg-exec-stub process and notifies cxxdbg process
    void set_pid(unsigned long proc_id, unsigned long thrd_id);
    
    /// Returns ID of main thread on Windows
    unsigned long tid();

    /// Waits for cxxdbg-exec-stub process ID specified amount of time.
    /// Returns pair of process ID and main thread ID (on Windows).
    std::tuple<unsigned long, unsigned long> wait_for_pid(const boost::posix_time::time_duration & tm);

    /// Waits for continue flag
    void wait_for_conitue();

    /// Sends continue signal
    void notify_continue();

private:
    /// Mutex for notifying cxxdbg process
    boost::interprocess::interprocess_mutex mutex_;

    /// Condition variable for notifying cxxdbg process about PID/TID
    boost::interprocess::interprocess_condition pid_cond_;

    unsigned long pid_;         ///< ID of cxxdbg-exec-stub process
    unsigned long tid_;         ///< ID of main thread on windows

    /// Condition variable for notifying debugee process about continue
    boost::interprocess::interprocess_condition continue_cond_;
};


}


