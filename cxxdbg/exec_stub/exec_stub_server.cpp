// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file exec_stub_server.cpp
/// Contains implementation of the exec_stub_server class.

#include "exec_stub_server.hpp"
#include "exec_stub_struct.hpp"
#include <sstream>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include "cxxdbg/boost_process/process.hpp"


namespace bp = boost::process;
namespace ip = boost::interprocess;


namespace cxxdbg::dbg::exec_stub {


/// Creates name for shared memory object
std::string make_shm_name() {
    std::ostringstream str;
    str << "cxxdbg_exec_stub_shm_" << boost::this_process::get_id();
    return str.str();
}


exec_stub_server::exec_stub_server() {
    std::string shm_name = make_shm_name();
    ip::shared_memory_object::remove(shm_name.c_str());
    shm_.reset(new ip::shared_memory_object(ip::create_only, shm_name.c_str(), ip::read_write));
    shm_->truncate(sizeof(exec_stub_struct));
    shm_reg_.reset(new ip::mapped_region(*shm_, ip::read_write));
    new (shm_reg_->get_address()) exec_stub_struct;
}


exec_stub_server::~exec_stub_server() {

    std::string shm_name = shared_mem_name();

    exec_stub_struct * str = static_cast<exec_stub_struct*>(shm_reg_->get_address());
    str->~exec_stub_struct();

    shm_reg_.reset();
    shm_.reset();
    ip::shared_memory_object::remove(shm_name.c_str());
}


std::string exec_stub_server::shared_mem_name() const {
    return shm_->get_name();
}


std::tuple<unsigned long, unsigned long> exec_stub_server::wait_pid(const std::chrono::nanoseconds & nsec) {
    exec_stub_struct * str = static_cast<exec_stub_struct*>(shm_reg_->get_address());
    std::chrono::microseconds msec =
            std::chrono::duration_cast<std::chrono::microseconds>(nsec);
    return str->wait_for_pid(boost::posix_time::microseconds(msec.count()));
}


void exec_stub_server::notify_continue() {
    exec_stub_struct * str = static_cast<exec_stub_struct*>(shm_reg_->get_address());
    str->notify_continue();
}


}
