// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file code_position.cpp
/// Contains implementation of code_position class.

#include "code_position.hpp"
#include "module.hpp"
#include "target_base.hpp"
#include "utils.hpp"


namespace cxxdbg::dbg::core {


code_position::code_position(target_base & t):
targ_{t} {
}


code_position::code_position(target_base & t, const lldb::SBAddress & addr):
targ_{t}, addr_(addr) {
    assert(addr.IsValid() && "Invalid lldb address");
}


code_position::code_position(const code_position & pos):
targ_{pos.targ_}, addr_(pos.addr_) {
}


code_position::~code_position(){
}


std::uint64_t code_position::load_addr() const {
    return lldb_addr().GetLoadAddress(targ_.lldb_targ());
}


std::uint64_t code_position::file_addr() const {
    return lldb_addr().GetFileAddress();
}


source_position code_position::get_src_pos() const {
    return source_position::from_line_entry(lldb_addr().GetLineEntry());
}


std::string code_position::get_func_name(bool get_params) const {
    return func_name_at_address(targ_.cm(), lldb_addr());
}


module code_position::get_module() const {
    return {lldb_addr().GetModule()};
}


code_position code_position::offset(long val) const {
    lldb::SBAddress addr(lldb_addr().GetSection(), lldb_addr().GetOffset());
    addr.OffsetAddress(val);
    return {targ_, addr};
}


code_position & code_position::operator=(const code_position & pos) {
    addr_ = pos.addr_;
    return *this;
}


code_position::operator bool() const {
    return lldb_addr().IsValid();
}


lldb::SBAddress code_position::lldb_addr() const {
    return addr_;
}


}
