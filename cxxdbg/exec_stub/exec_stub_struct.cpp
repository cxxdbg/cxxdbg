// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file exec_stub_struct.cpp
/// Contains implementation of the exec_stub_struct class.

#include "exec_stub_struct.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include <iomanip>


namespace ip = boost::interprocess;
namespace dt = boost::date_time;


namespace cxxdbg::dbg::exec_stub {


exec_stub_struct::exec_stub_struct():
pid_(0), tid_(0) {
}


exec_stub_struct::~exec_stub_struct() {
}


unsigned long exec_stub_struct::pid() {
    ip::scoped_lock<ip::interprocess_mutex> lock(mutex_);
    return pid_;
}


void exec_stub_struct::set_pid(unsigned long proc_id, unsigned long thrd_id) {
    ip::scoped_lock<ip::interprocess_mutex> lock(mutex_);
    pid_ = proc_id;
    tid_= thrd_id;
    pid_cond_.notify_all();
}


unsigned long exec_stub_struct::tid() {
    ip::scoped_lock<ip::interprocess_mutex> lock(mutex_);
    return tid_;
}


std::tuple<unsigned long, unsigned long> exec_stub_struct::wait_for_pid(const boost::posix_time::time_duration & tm) {
    ip::scoped_lock<ip::interprocess_mutex> lock(mutex_);
    pid_cond_.timed_wait(lock, boost::posix_time::microsec_clock::universal_time() + tm);
    return {pid_, tid_};
}


void exec_stub_struct::wait_for_conitue() {
    ip::scoped_lock<ip::interprocess_mutex> lock(mutex_);
    continue_cond_.wait(lock);
}


void exec_stub_struct::notify_continue() {
    continue_cond_.notify_all();
}


}
