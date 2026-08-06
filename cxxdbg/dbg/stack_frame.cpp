// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file stack_frame.cpp
/// Contains implementation of frame_entry class.

#include "stack_frame.hpp"
#include "cxxdbg/util/convert.hpp"
#include <iomanip>


namespace cxxdbg::dbg {


stack_frame::stack_frame(const code_position &p, uint64_t pcfa, uint64_t psctx):
pos_(p),
cfa_{pcfa},
sctx_{psctx} {
}


const code_position & stack_frame::pos() const {
    return pos_;
}


uint64_t stack_frame::cfa() const {
    return cfa_;
}


uint64_t stack_frame::sctx() const {
    return sctx_;
}


stack_frame::call_parameter_vector & stack_frame::params() {
    return params_;
}


const stack_frame::call_parameter_vector & stack_frame::params() const {
    return params_;
}


void stack_frame::add_param(const call_parameter & par) {
    params_.push_back(par);
}


/// Prints uint64_t to output stream in hex
static void print_hex_uint64_t(std::wostream & str, uint64_t val) {
    str << "0x" << std::setw(16) << std::setfill(L'0') << std::hex << val;
}


/// Converts string to unsigned long, returns true if conversion is possible
/// TODO: rewrite this
bool string_to_unsigned_long(const std::string & str, unsigned long & val) {
    try {
        std::size_t last_idx;
        unsigned long res = std::stoul(str, &last_idx);
        if (last_idx == str.size()) {
            return res;
        }
    }
    catch(std::exception &) {
    }

    return false;
}


void stack_frame::print(std::wostream & str,
                        bool display_param_types,
                        bool display_param_names,
                        bool display_param_vals) const {

    if (pos().func_name().empty()) {
        // function name is not available, printing function address
        print_hex_uint64_t(str, pos().load_addr());
        return;
    }

    // printing function name
    str << util::convert::to_wstring(pos().func_name());

    if (!display_param_names &&
        !display_param_types &&
        !display_param_vals)
        return;

    // printing parameters

    str << "(";
    bool first = true;

    for (auto it = params().begin(), end = params().end(); it != end; ++it) {

        // printing parameter separator if needed
        if (!first) {
            str << ", ";
        } else {
            first = false;
        }

        bool first_item = true;

        // printing parameter type
        if (display_param_types) {
            str << it->type();
            first_item = false;
        }

        // printing parameter name
        if (display_param_names) {
            if (!first_item)
                str << " ";

            str << it->name();
        }

        // printing parameter value
        if (display_param_vals) {
            if (display_param_types || display_param_names) {
                str << " = ";
            }

            str << it->value();
        }
    }

    str << ")";
}


}
