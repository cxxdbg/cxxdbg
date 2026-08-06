// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file function_breakpoint.cpp
/// Contains implementation of function_breakpoint class.

#include "function_breakpoint.hpp"
#include "breakpoint_list_impl.hpp"


namespace cxxdbg::dbg {


function_breakpoint::function_breakpoint(source_model & smdl, id_type i, const std::string & nm):
code_breakpoint{smdl, i},
func_name_{nm} {
}


function_breakpoint::~function_breakpoint() {
}


const std::string & function_breakpoint::func_name() const {
    return func_name_;
}


void function_breakpoint::set_func_name(const std::string & nm) {
    func_name_ = nm;
}


std::string function_breakpoint::name() const {
    return func_name();
}


void function_breakpoint::install_into_impl(breakpoint_list_impl & impl,
                                            const install_handler & h) {
    impl.add_breakpoint(func_name(), enabled(), condition(), h);
}


}
