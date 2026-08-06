// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file exec_stub_server.hpp
/// Contains definition of the exec_stub_server class.

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <boost/interprocess/interprocess_fwd.hpp>


namespace cxxdbg::dbg::exec_stub {


/// \class exec_stub_server
/// Server class which manages and initializes shared memory region
/// and waits for PID of cxxdbg-exec-stub process. Used in debugger
class exec_stub_server {
public:
    /// Constructor, makes shared memory region and initializes
    /// exec_stub struct.
    exec_stub_server();

    /// Destructor, destroys shared memory region
    ~exec_stub_server();

    /// Returns name of memory region
    std::string shared_mem_name() const;

    /// Waits for cxxdbg-exec-stub PID specified amount of time.
    /// Returns pair of process ID and main thread ID (on Windows)
    std::tuple<unsigned long, unsigned long> wait_pid(const std::chrono::nanoseconds & nsec);

    /// Notifies debugee process that is should continue execution
    void notify_continue();

private:
    std::shared_ptr<boost::interprocess::shared_memory_object> shm_;
    std::shared_ptr<boost::interprocess::mapped_region> shm_reg_;
};


}


