// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file code_position_info.cpp
/// Contains implementation of code_position_info class.

#include "code_position_info.hpp"


namespace cxxdbg::dbg {



code_position_info::code_position_info(std::uint64_t laddr,
                                       std::uint64_t faddr,
                                       const std::string & fname,
                                       const source_position_info & spos):
load_addr_(laddr),
file_addr_(faddr),
func_name_(fname),
src_pos_(spos) {
}


std::uint64_t code_position_info::load_addr() const {
    return load_addr_;
}


std::uint64_t code_position_info::file_addr() const {
    return file_addr_;
}


const std::string & code_position_info::func_name() const {
    return func_name_;
}


const source_position_info & code_position_info::src_pos() const {
    return src_pos_;
}


}
