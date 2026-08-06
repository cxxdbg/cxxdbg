// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file instruction.cpp
/// Contains implementation of the instruction class.

#include "instruction.hpp"
#include "cxxdbg/util/print.hpp"
#include <sstream>


namespace cxxdbg::dbg::core {


bool instruction::operator==(const instruction & other) const {
    if (addr() != other.addr() || string() != other.string()) {
        return false;
    }

    if (data().size() != other.data().size()) {
        return false;
    }

    for (int i = 0, sz = data().size(); i < sz; ++i) {
        if (data()[i] != other.data()[i]) {
            return false;
        }
    }

    if (symbol() != other.symbol()) {
        return false;
    }

    if (src_pos() != other.src_pos()) {
        return false;
    }

    return true;
}


std::string instruction::addr_str(size_t ptr_size) const {
    std::ostringstream str;
    util::print_hex(str, addr(), sizeof(uint64_t), ptr_size);
    return str.str();
}


std::string instruction::data_str() const {
    std::ostringstream str;
    bool first = true;

    for (char c : data()) {
        if (first) {
            first = false;
        } else {
            str << ' ';
        }

        auto num = static_cast<unsigned int>(static_cast<unsigned char>(c));
        util::print_hex_no_prefix(str, num, sizeof(num), 1);
    }

    return str.str();
}


}
