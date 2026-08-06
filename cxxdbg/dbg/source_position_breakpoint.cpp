// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file source_position_breakpoint.cpp
/// Contains implementation of source_position_breakpoint class.

#include "source_position_breakpoint.hpp"
#include "breakpoint_list_impl.hpp"
#include <sstream>


namespace cxxdbg::dbg {


source_position_breakpoint::source_position_breakpoint(source_model & smdl,
                                                       id_type i,
                                                       const source_position_info & pos):
code_breakpoint{smdl, i},
pos_{pos} {
    assert(pos_ && "source position should be valid");
}


source_position_breakpoint::~source_position_breakpoint() {
}


const source_position_info & source_position_breakpoint::pos() const {
    return pos_;
}


void source_position_breakpoint::set_pos(const source_position_info & p) {
    pos_ = p;
}


std::string source_position_breakpoint::name() const {
    std::ostringstream str;
    str << pos().path().string() << ", line " << pos().line();
    return str.str();
}


void source_position_breakpoint::install_into_impl(breakpoint_list_impl & impl,
                                                   const install_handler & h) {
    impl.add_breakpoint(pos(), enabled(), condition(), h);
}


}
