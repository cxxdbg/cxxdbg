// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file lldb_disassembler.cpp
/// Contains implementation of the lldb_disassembler class.

#include "lldb_disassembler.hpp"
#include "target_base.hpp"
#include <lldb/Target/Target.h>
#include <lldb/API/SBInstruction.h>
#include <sstream>


namespace cxxdbg::dbg::core {


lldb_disassembbler::lldb_disassembbler(target_base & targ):
targ_{targ} {
}


lldb_disassembbler::~lldb_disassembbler() = default;


size_t lldb_disassembbler::max_inst_byte_size() const {
    return static_cast<size_t>(targ_.lldb_targ().GetSP()->GetArchitecture().GetMaximumOpcodeByteSize());
}


/// Converts SBInstruction to instruction
static instruction convert_instruction(lldb::SBInstruction inst,
                                       lldb::SBTarget targ) {

    auto inst_addr = inst.GetAddress().GetLoadAddress(targ);
    auto inst_size = inst.GetByteSize();

    // read instruction data
    auto data = inst.GetData(targ);
    assert(data.IsValid() && "can't get instruction data");
    assert(data.GetByteSize() == inst_size && "invalid instruction data byte size");
    std::vector<char> inst_data;
    inst_data.resize(inst_size);
    lldb::SBError err;
    auto read_size = data.ReadRawData(err, 0, inst_data.data(), inst_size);
    if (read_size != inst_size) {
        return {};
    }

    // reading instruction text representation

    std::ostringstream inst_str;

    if (auto mnem = inst.GetMnemonic(targ)) {
        inst_str << mnem;
    }

    if (auto ops = inst.GetOperands(targ)) {
        std::string ops_str = ops;
        ops_str.erase(std::remove_if(ops_str.begin(), ops_str.end(), isspace), ops_str.end());
        if (!ops_str.empty()) {
            inst_str << ' ' << ops_str;
        }
    }

    if (auto comment = inst.GetComment(targ)) {
        std::string comment_str = comment;
        comment_str.erase(std::remove_if(comment_str.begin(), comment_str.end(), isspace), comment_str.end());
        if (!comment_str.empty()) {
            inst_str << "; " << comment_str;
        }
    }

    // creating instruction instance
    instruction res_inst{inst_addr, std::move(inst_data), inst_str.str()};

    // reading instruction line entry
    auto sc = inst.GetAddress().GetSymbolContext(lldb::eSymbolContextLineEntry);
    if (sc.IsValid()) {
        auto line_entry = sc.GetLineEntry();
        if (line_entry.IsValid()) {
            auto src_pos = source_position::from_line_entry(line_entry);
            res_inst.set_src_pos(src_pos);
        }
    }

    return res_inst;
}


std::vector<instruction> lldb_disassembbler::disassemble(uint64_t start_addr, uint64_t size) const {
    assert(size != 0 && "region size should not be zero");
    assert(start_addr <= SIZE_MAX - size && "size of disassemble region is beyond the end of memory");
    auto end_addr = start_addr + size;

    // reading instructions
    auto sbaddr = targ_.lldb_targ().ResolveLoadAddress(start_addr);
    auto insts = targ_.lldb_targ().ReadInstructions(sbaddr, size);

    std::vector<instruction> res;
    source_position prev_src_pos;

    // converting instructions until end address is reached
    for (uint32_t i = 0, sz = insts.GetSize(); i < sz; ++i) {
        auto sbinst = insts.GetInstructionAtIndex(i);

        // converting instruction
        auto inst = convert_instruction(sbinst, targ_.lldb_targ());

        // removing source position if it's same as source position of previous instruction
        if (inst.src_pos() == prev_src_pos) {
            inst.set_src_pos({});
        } else {
            prev_src_pos = inst.src_pos();
        }

        if (inst.end_addr() > end_addr) {
            // instruction end address is after requested end address
            // removing exceed data from instruction and replacing string representation
            inst.data().resize(end_addr - inst.addr());
            inst.set_string("<invalid instruction>");
        }

        res.push_back(inst);

        // checking of instruction end address
        if (inst.end_addr() == end_addr) {
            break;
        }
    }

    // if not all instructions were read then filling missing instructions with
    // 4-byte memory chunks without string representation
    auto inst_end_addr = res.empty() ? start_addr : res.back().end_addr();
    if (inst_end_addr < end_addr) {
        auto curr_addr = inst_end_addr;
        while (true) {
            auto inst_size = end_addr - curr_addr;
            if (inst_size > 4) {
                inst_size = 4;
            }

            std::vector<char> data;
            data.resize(inst_size);
            auto sbaddr = targ_.lldb_targ().ResolveLoadAddress(curr_addr);
            lldb::SBError err;
            targ_.lldb_targ().ReadMemory(sbaddr, data.data(), data.size(), err);

            res.push_back(instruction{curr_addr, data, {}});

            // checking for end of memory block
            // NOTE: avoid just adding step to address because it will fail for values near UINT64_MAX
            
            if (inst_size < 4) {
                break;
            }

            curr_addr += 4;

            if (curr_addr == end_addr) {
                break;
            }
        }
    }

    assert(!res.empty() && "instruction list should not be empty");
    assert(res.back().end_addr() == end_addr && "invalid end address for last instruction");

    return res;
}


size_t lldb_disassembbler::ptr_size() const {
    return targ_.ptr_byte_size();
}


disassembler_object lldb_disassembbler::obj_at_addr(uint64_t addr) const {
    auto sbaddr = targ_.lldb_targ().ResolveLoadAddress(addr);

    // resolving symbol at address
    auto ctx = sbaddr.GetSymbolContext(lldb::eSymbolContextSymbol);

    if (!ctx.IsValid()) {
        // can't get symbol context for address
        return {};
    }

    auto sym = ctx.GetSymbol();
    if (!sym.IsValid()) {
        return {};
    }

    auto start_addr = sym.GetStartAddress().GetLoadAddress(targ_.lldb_targ());
    auto end_addr = sym.GetEndAddress().GetLoadAddress(targ_.lldb_targ());
    return {static_cast<uint64_t>(start_addr), static_cast<size_t>(end_addr - start_addr), sym.GetName()};
}


}
