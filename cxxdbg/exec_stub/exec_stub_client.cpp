// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file exec_stub_client.cpp
/// Contains implementation of the exec_stub_client class.

#include "exec_stub_client.hpp"
#include "exec_stub_struct.hpp"
#include "cxxdbg/boost_process/process.hpp"


namespace ip = boost::interprocess;


namespace cxxdbg::dbg::exec_stub {


exec_stub_client::exec_stub_client(const std::string & shm_name):
shm_(ip::open_only, shm_name.c_str(), ip::read_write),
shm_reg_(shm_, ip::read_write) {
}


exec_stub_client::~exec_stub_client() {
}


void exec_stub_client::send_pid(unsigned long pid, unsigned long tid) {
    exec_stub_struct * str = static_cast<exec_stub_struct*>(shm_reg_.get_address());
    str->set_pid(pid, tid);
}


void exec_stub_client::send_pid() {
    send_pid(boost::this_process::get_id(), 0);
}


void exec_stub_client::wait_continue() {
    exec_stub_struct * str = static_cast<exec_stub_struct*>(shm_reg_.get_address());
    str->wait_for_conitue();
}


}
