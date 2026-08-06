// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file stack_frame_info.cpp
/// Contains implementation of stack_frame_info class.

#include "stack_frame_info.hpp"


namespace cxxdbg::dbg {


stack_frame_info::stack_frame_info(const code_position_info & sp,
                                   uint64_t cfap,
                                   uint64_t s_id):
pos_(sp),
cfa_{cfap},
sc_id_{s_id} {
}


const code_position_info & stack_frame_info::pos() const {
    return pos_;
}


uint64_t stack_frame_info::cfa() const {
    return cfa_;
}


uint64_t stack_frame_info::sc_id() const {
    return sc_id_;
}


const stack_frame_info::call_parameter_info_vector & stack_frame_info::params() const {
    return params_;
}


void stack_frame_info::add_param(const call_parameter_info & par) {
    params_.push_back(par);
}


}
