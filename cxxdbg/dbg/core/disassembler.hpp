// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file disassembler.hpp
/// Contains definition of the abstract disassembler class.

#pragma once

#include "instruction.hpp"
#include <memory>
#include <string>
#include <vector>


namespace cxxdbg::dbg::core {


/// Represents an object in memory for disassembler
class disassembler_object {
public:
    /// Constructs invalid object
    disassembler_object():
        addr_{UINT64_MAX}, size_{SIZE_MAX} {}

    /// Constructs object with specified address, size and name
    disassembler_object(uint64_t a, size_t sz, const std::string & nm):
        addr_{a}, size_{sz}, name_{nm} {}
    
    /// Returns true if object is valid
    bool is_valid() const { return addr_ != UINT64_MAX || size_ != SIZE_MAX; }

    /// Returns object start address
    uint64_t addr() const { return addr_; }

    /// Returns object size
    size_t size() const { return size_; }

    /// Returns object name
    const std::string & name() const { return name_; }

private:
    uint64_t addr_;         ///< Object address
    size_t size_;           ///< Object size
    std::string name_;      ///< Object name
};


/// Abstract disassembler that disassembles single continuous piece of memory
/// and have access to information about memory objeccts.
class disassembler {
public:
    /// Default virtual destructor
    virtual ~disassembler() = default;

    /// Returns maximum size of instruction in bytes
    virtual size_t max_inst_byte_size() const = 0;

    /// Returns pointer (word) size for target architecture
    virtual size_t ptr_size() const = 0;

    /// Disassembles memory into list of instructions
    virtual std::vector<instruction> disassemble(uint64_t start_addr, uint64_t size) const = 0;

    /// Returns object located at specified address or invalid object
    virtual disassembler_object obj_at_addr(uint64_t addr) const = 0;
};


}
