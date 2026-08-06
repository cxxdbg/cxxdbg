// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file breakpoint_location.cpp
/// Contains implementation of breakpoint_location class.

#include "breakpoint_location.hpp"


namespace cxxdbg::dbg {


breakpoint_location::breakpoint_location(code_breakpoint * bp, id_type i, const code_position & p):
bp_(bp),
id_(i),
pos_(p) {
}


breakpoint_location::~breakpoint_location() {
}


const code_breakpoint * breakpoint_location::bp() const {
    return bp_;
}


code_breakpoint * breakpoint_location::bp() {
    return bp_;
}


breakpoint_location::id_type breakpoint_location::id() const {
    return id_;
}


const code_position & breakpoint_location::pos() const {
    return pos_;
}


void breakpoint_location::set_pos(const code_position & p) {
    pos_ = p;
}

const breakpoint_site * breakpoint_location::child_at(std::size_t position) const { return nullptr; }

std::size_t breakpoint_location::children_size() const { return 0; }

bool breakpoint_location::can_change_enabled_state() const { return false; }

const breakpoint_location * breakpoint_location::get_single_location() const {
    return this;
}

source_position breakpoint_location::get_source_position() const {
    return pos_.src_pos();
}


}

