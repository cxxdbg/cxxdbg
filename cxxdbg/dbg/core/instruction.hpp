// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file instruction.hpp
/// Contains definition of the instruction class

#pragma once

#include "source_position.hpp"
#include <cassert>
#include <string>
#include <vector>


namespace cxxdbg::dbg::core {


/// Represents single disassembled instruction
class instruction {
public:
    /// Constructs invalid instruction
    instruction(): addr_{0} {}

    /// Constructs instruction
    instruction(uint64_t a, std::vector<char> d, std::string str, std::string sym = {}):
    addr_{a}, data_{std::move(d)}, str_{std::move(str)}, sym_{std::move(sym)} {
        assert(data_.size() != 0 && "instruction data must not be empty");
    }

    /// Returns true if instruction is valid
    bool is_valid() const { return data_.size() != 0; }

    /// Returns instruction address
    uint64_t addr() const { return addr_; }

    /// Returns instruction data
    auto & data() { return data_; }

    /// Returns instruction data
    auto & data() const { return data_; }

    /// Returns instruction end address
    uint64_t end_addr() const {
        assert(addr() <= UINT64_MAX - data().size() && "end of instruction is greater than UINT64_MAX");
        return addr() + data().size();
    }

    /// Returns instruction string representation
    auto & string() const { return str_; }

    /// Sets instruction string representation
    void set_string(std::string s) {
        str_ = std::move(s);
    }

    /// Returns true if this instruction is equal to another
    bool operator==(const instruction & other) const;

    /// Returns instruction address as string representation
    std::string addr_str(size_t ptr_size) const;

    /// Returns instruction data as string representation
    std::string data_str() const;

    /// Returns instruction starting symbol
    auto & symbol() const { return sym_; }

    /// Sets instruction starting symbol
    void set_symbol(std::string & s) { sym_ = std::move(s); }

    /// Returns instruction source position
    auto & src_pos() const { return src_pos_; }

    /// Sets instruction source position
    void set_src_pos(const source_position & sp) { src_pos_ = sp; }

private:
    uint64_t addr_;             ///< Instruction address
    std::vector<char> data_;    ///< Instruction bytes
    std::string str_;           ///< Instruction string representation
    std::string sym_;           ///< Instruction starting symbol
    source_position src_pos_;   ///< Instruction source position
};


}
