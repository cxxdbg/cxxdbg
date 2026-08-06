// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread_list_info.cpp
/// Contains implementation of thread_list_info class.

#include "thread_list_info.hpp"
#include <cassert>


namespace cxxdbg::dbg {


const thread_list_info::size_type thread_list_info::invalid_index;


thread_list_info::thread_list_info():
active_thread_(invalid_index) {
}


thread_list_info::const_iterator thread_list_info::begin() const {
    return threads_.begin();
}


thread_list_info::const_iterator thread_list_info::end() const {
    return threads_.end();
}


thread_list_info::size_type thread_list_info::current_thread() const {
    return active_thread_;
}


void thread_list_info::add(const thread_info & thrd) {
    threads_.push_back(thrd);
}


void thread_list_info::set_current_thread(size_type i) {
    assert(i < threads_.size() && "Invalid index of active thread");
    active_thread_ = i;
}

const thread_info & thread_list_info::operator[](thread_list_info::size_type i) const {
    assert(i < threads_.size() && "Invalid index of thread");
    return threads_[i];
}


}
