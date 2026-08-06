// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread_info.cpp
/// Contains implementation of thread_info class.

#include "thread_info.hpp"


namespace cxxdbg::dbg {


thread_info::thread_info(std::uint64_t i):
id_{i},
is_complete_call_stack_{true} {
}


std::uint64_t thread_info::id() const {
    return id_;
}


const thread_info::stack_frame_info_vector & thread_info::call_stack() const {
    return call_stack_;
}


thread_info::const_stack_frame_iterator thread_info::call_stack_begin() const {
    return call_stack_.begin();
}


thread_info::const_stack_frame_iterator thread_info::call_stack_end() const {
    return call_stack_.end();
}


size_t thread_info::call_stack_size() const {
    return call_stack_.size();
}


void thread_info::add_stack_frame(const stack_frame_info & finfo) {
    call_stack_.push_back(finfo);
}


bool thread_info::has_complete_call_stack() const {
    return is_complete_call_stack_;
}


void thread_info::set_has_complete_call_stack(bool v) {
    is_complete_call_stack_ = v;
}


size_t thread_info::curr_frame_index() const {
    return curr_frame_;
}


void thread_info::set_curr_frame_index(size_t idx) {
    curr_frame_ = idx;
}


}
