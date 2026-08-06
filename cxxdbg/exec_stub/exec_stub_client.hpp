// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file exec_stub_client.hpp
/// Contains definition of the exec_stub_client class.

#pragma once

#include <string>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>


namespace cxxdbg::dbg::exec_stub {


/// \class exec_stub_client
/// Client class which opens shared memory object and sends
/// PID of cxxdbg-exec-stub executable to a server (debugger)
class exec_stub_client {
public:
    /// Constructor, opens shared memory object with specified name
    exec_stub_client(const std::string & shm_name);

    /// Destructor, closes shared memory object
    ~exec_stub_client();

    /// Sends PID to exec_stub server
    void send_pid(unsigned long pid, unsigned long tid = 0);

    /// Sends PID of current process to exec_stub server
    void send_pid();

    /// Waits for continue notification from server
    void wait_continue();

private:
    boost::interprocess::shared_memory_object shm_;
    boost::interprocess::mapped_region shm_reg_;
};


}


