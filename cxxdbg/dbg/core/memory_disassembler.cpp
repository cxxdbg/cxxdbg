// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file memory_disassembler.cpp
/// Contains implementation of the memory_disassembler class.

#include "memory_disassembler.hpp"
#include <algorithm>


namespace cxxdbg::dbg::core {


std::vector<instruction> memory_disassembler::disassemble(uint64_t addr, int64_t inst_offset, uint64_t inst_count) const {
    // calculating symbol lookup limits. It should be greater than default lookup limit
    // for big values of instruction count.
    auto symbol_lookup_limit = std::max(def_symbol_lookup_limit, inst_count * dis_.max_inst_byte_size() * 2);

    // calculating minimum start address for disassembly
    auto start_addr = addr;
    if (inst_offset >= 0) {
        // using addr as start address
    } else {
        auto addr_offset = (-inst_offset) * dis_.max_inst_byte_size();
        if (addr_offset <= start_addr) {
            start_addr -= addr_offset;
        } else {
            start_addr = 0;
        }
    }

    // result list of disassembled instructions
    std::vector<instruction> insts;

    // index of instruction at address passed in the addr parameter
    size_t addr_inst_idx = SIZE_MAX;

    // disassembling memory regions from start address
    auto curr_addr = start_addr;
    while (true) {
        // dissassembling current memory region
        auto [reg_start_addr, reg_end_addr, obj_name] = find_mem_region(curr_addr, symbol_lookup_limit);
        assert(reg_start_addr < reg_end_addr && "invalid memory region");
        assert(reg_start_addr <= curr_addr && "invalid memory region");
        assert(curr_addr <= reg_end_addr && "invalid memory region");

        auto reg_insts = dis_.disassemble(reg_start_addr, reg_end_addr - reg_start_addr);

        curr_addr = reg_end_addr;

        assert(!reg_insts.empty() && "instruction list is empty");

        // checking for consistency of addresses and sizes of instructions
        size_t curr_inst_addr = reg_insts.front().addr();
        for (auto && inst : reg_insts) {
            assert(inst.addr() == curr_inst_addr && "invalid instruction address");
            curr_inst_addr += inst.data().size();
        }

        // searching for instruction located at addr
        if (addr_inst_idx == SIZE_MAX && addr < reg_insts.back().end_addr()) {
            auto addr_inst_it = std::find_if(reg_insts.begin(), reg_insts.end(), [addr](auto && inst) {
                return inst.addr() <= addr && addr < inst.end_addr();
            });

            if (addr_inst_it != reg_insts.end()) {
                addr_inst_idx = insts.size() + (addr_inst_it - reg_insts.begin());
            }
        }

        // setting symbol name for the first instruction
        reg_insts.front().set_symbol(obj_name);

        // appending instructions to the result list
        std::copy(reg_insts.begin(), reg_insts.end(), std::back_inserter(insts));

        // adjusting instruction offset in case of the begginning of memory
        if (addr_inst_idx != SIZE_MAX) {
            if (inst_offset < 0 && -inst_offset > addr_inst_idx) {
                assert(!insts.empty() && "list of instructions should not be empty here");
                assert(insts.front().addr() == 0 && "not all required instructions were disassembled");
                inst_offset = -static_cast<int64_t>(addr_inst_idx);
            }
        }

        // checking that required number of instructions were diassembled
        if (addr_inst_idx != SIZE_MAX) {
            if (inst_offset < 0) {
                assert(-inst_offset <= addr_inst_idx && "instructions at required offset were not disassembled");
            }

            assert(addr_inst_idx < insts.size() && "invalid instruction index");
            if (addr_inst_idx + inst_offset + inst_count <= insts.size()) {
                // all required instructions were diassembled
                break;
            }
        }

        // checking of the end of memory
        if (reg_end_addr == UINT64_MAX) {
            break;
        }
    }

    assert(!insts.empty() && "instruction list should not be empty here");

    if (inst_offset < 0) {
        assert(-inst_offset <= addr_inst_idx && "instructions at required offset were not disassembled");
    }

    // removing excess instructions from the end of instructions list
    if (addr_inst_idx + inst_offset + inst_count <= insts.size()) {
        insts.erase(insts.begin() + addr_inst_idx + inst_offset + inst_count, insts.end());
    } else {
        assert(insts.back().end_addr() == UINT64_MAX && "not all instructions were disassembled");
    }

    // removing excess instructions at the beginning of instructions list
    assert(addr_inst_idx + inst_offset <= insts.size() && "not all required instructions were disassembled");
    insts.erase(insts.begin(), insts.begin() + addr_inst_idx + inst_offset);

    return insts;
}


std::tuple<uint64_t, uint64_t, std::string>
memory_disassembler::find_mem_region(uint64_t addr, size_t symbol_lookup_limit) const {
    assert(addr != UINT64_MAX && "invalid region address");

    // checking if address is inside memory object
    auto obj = dis_.obj_at_addr(addr);
    if (obj.is_valid()) {
        // beginning of the object is the start of memory region
        assert(obj.addr() <= addr && addr <= obj.addr() + obj.size() && "invlalid object at address");
        return {obj.addr(), obj.addr() + obj.size(), obj.name()};
    }

    size_t step = 1;

    // trying find memory object which is located before memory address
    uint64_t start_addr = addr;
    while (true) {
        if (start_addr < step) {
            // beginning of memory is reached
            start_addr = 0;
            break;
        }

        if (addr - start_addr > symbol_lookup_limit) {
            // lookup limit is reached
            break;
        }

        start_addr -= step;

        auto obj = dis_.obj_at_addr(start_addr);
        if (obj.is_valid()) {
            // end of the object is the start of memory region
            start_addr = obj.addr() + obj.size();
            assert(start_addr <= addr && "invalid object end address");
            break;
        }
    }

    // trying find memory object which is located after memory address
    uint64_t end_addr = addr;
    while (true) {
        if (end_addr > UINT64_MAX - step) {
            // end of memory is reached
            end_addr = UINT64_MAX;
            break;
        }

        if (end_addr - addr > symbol_lookup_limit) {
            // lookup limit is reached
            break;
        }

        end_addr += step;

        auto obj = dis_.obj_at_addr(end_addr);
        if (obj.is_valid()) {
            // start of the object is the end of memory region
            end_addr = obj.addr();
            assert(end_addr > addr && "invalid object start address");
            break;
        }
    }

    assert((start_addr <= addr) && "invalid memory region start address");
    assert(end_addr > addr && "invalid memory region end address");

    return {start_addr, end_addr, {}};
}


}
