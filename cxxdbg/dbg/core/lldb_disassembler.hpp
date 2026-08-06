// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file lldb_disassembler.hpp
/// Contains definition of the lldb_disassembler class.

#pragma once

#include "disassembler.hpp"


namespace cxxdbg::dbg::core {

class target_base;


class lldb_disassembbler: public disassembler {
public:
    /// Constructs disassembler for specified reference to target
    lldb_disassembbler(target_base & targ);

    /// Default virtual destructor
    ~lldb_disassembbler() override;

    /// Returns maximum size of instruction in bytes
    size_t max_inst_byte_size() const override;

    /// Disassembles memoty into list of instructions
    std::vector<instruction> disassemble(uint64_t start_addr, uint64_t size) const override;

    /// Returns pointer (word) size for target architecture
    size_t ptr_size() const override;

    /// Returns object located at specified address or invalid object
    disassembler_object obj_at_addr(uint64_t addr) const override;

private:
    target_base & targ_;                ///< Reference to target
};


}
