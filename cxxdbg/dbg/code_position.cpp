// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file code_position.cpp
/// Contains implementation of code_postion class.

#include "code_position.hpp"
#include "code_position_info.hpp"
#include "source_model.hpp"
#include "cxxdbg/util/print.hpp"
#include <iomanip>
#include <sstream>


namespace cxxdbg::dbg {


code_position::code_position(std::uint64_t laddr,
                             std::uint64_t faddr,
                             const std::string & fname,
                             const source_position & spos):
load_addr_{laddr},
file_addr_{faddr},
func_name_{fname},
src_pos_{spos} {
}


code_position::code_position(const code_position_info & pinfo, source_model & sources):
load_addr_{pinfo.load_addr()},
file_addr_{pinfo.file_addr()},
func_name_{pinfo.func_name()},
src_pos_{pinfo.src_pos(), sources} {
}


std::uint64_t code_position::load_addr() const {
    return load_addr_;
}


bool code_position::has_load_addr() const {
    return load_addr_ != UINT64_MAX;
}


std::uint64_t code_position::file_addr() const {
    return file_addr_;
}


std::uint64_t code_position::addr() const {
    if (has_load_addr())
        return load_addr();

    return file_addr();
}


std::string code_position::addr_str() const {
    std::ostringstream str;
    util::print_hex(str, addr(), 8, 4);
    return str.str();
}


const std::string & code_position::func_name() const {
    return func_name_;
}


const source_position & code_position::src_pos() const {
    return src_pos_;
}


std::string code_position::src_pos_str() const {
    // returning source position string representation if source position is valid
    if (src_pos())
        return src_pos().string();

    // returning function name if function name is valid
    if (!func_name().empty())
        return func_name();

    // use address as source position
    return addr_str();
}


bool code_position::operator==(const code_position & pos) const {
    return load_addr() == pos.load_addr() &&
           file_addr() == pos.file_addr() &&
           func_name() == pos.func_name() &&
           src_pos() == pos.src_pos();
}


bool code_position::operator!=(const code_position & pos) const {
    return !(*this == pos);
}


}
