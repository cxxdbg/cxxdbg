// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file memory_disassembler.hpp
/// Contains definition of the memory disassembler_class and related classes.

#pragma once

#include "disassembler.hpp"


namespace cxxdbg::dbg::core {


/// Disassembler that disassembles memory region taking into account symbols layout
class memory_disassembler {
    /// Limit for symbol lookup before or after the address to disassemble
    static constexpr uint64_t def_symbol_lookup_limit = 4096;

    /// Symbol size limit to disassemble: 1Mb
    static constexpr uint64_t symbol_size_limit = 1024 * 1024;

public:
    /// Constructs disassembler with specified reference to simple diassembler
    memory_disassembler(disassembler & dis):
        dis_{dis} {}

    /// Disassembles memory at specified address with specified instruction offset
    /// and instruction count
    std::vector<instruction> disassemble(uint64_t addr, int64_t inst_offset, uint64_t inst_count) const;

private:
    /// Searches for the beginning of memory region for specified address. Returns tuple containing
    /// start and end address of memory region, and name of memory object if memory region is an object.
    std::tuple<uint64_t, uint64_t, std::string> find_mem_region(uint64_t addr, size_t symbol_lookup_limit) const;

    disassembler & dis_;                ///< Reference to simple disassembler
};


}
