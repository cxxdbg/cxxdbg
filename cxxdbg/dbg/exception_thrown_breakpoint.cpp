// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

//
// Created by yzarudny on 21.10.17.
//

#include "exception_thrown_breakpoint.hpp"
#include "breakpoint_list_impl.hpp"

namespace cxxdbg::dbg {


exception_thrown_breakpoint::exception_thrown_breakpoint(source_model & smdl, num_t i):
code_breakpoint{smdl, i} {
}


exception_thrown_breakpoint::~exception_thrown_breakpoint() {
}


std::string exception_thrown_breakpoint::name() const {
    return "Exception thrown";
}


void exception_thrown_breakpoint::install_into_impl(breakpoint_list_impl & impl,
                                                    const install_handler & h) {
    impl.add_breakpoint(function_name(), enabled(), condition(), h);
}


}
