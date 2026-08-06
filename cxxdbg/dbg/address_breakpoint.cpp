// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file address_breakpoint.cpp
/// Contains implementation of the address_breakpoint class.

#include "address_breakpoint.hpp"
#include "breakpoint_list_impl.hpp"
#include "cxxdbg/util/print.hpp"
#include <sstream>


namespace cxxdbg::dbg {


address_breakpoint::address_breakpoint(source_model & smdl, id_type i, std::uint64_t addr):
code_breakpoint{smdl, i},
addr_{addr} {
}


address_breakpoint::~address_breakpoint() {
}


std::uint64_t address_breakpoint::address() const {
    return addr_;
}


void address_breakpoint::set_address(uint64_t addr) {
    addr_ = addr;
}


std::string address_breakpoint::name() const {
    std::ostringstream str;
    // TODO: get address size from platform properties
    util::print_hex(str, addr_, 8, 8);
    return str.str();
}


void address_breakpoint::install_into_impl(breakpoint_list_impl & impl,
                                           const install_handler & h) {
    impl.add_breakpoint(addr_, enabled(), condition(), h);
}


}
