// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file lldb_memory_storage.cpp
/// Contains implementation of the lldb_memory_storage class.

#include "lldb_memory_storage.hpp"
#include "dbgfmt/data/error.hpp"
#include "cxxdbg/util/print.hpp"
#include <iostream>


namespace cxxdbg::dbg::core {


void lldb_memory_storage::read(void * data, size_t sz, uint64_t offset) {
    lldb::SBError err;
    lldb::SBAddress addr{offset, targ_};

    auto nread = targ_.ReadMemory(addr, data, sz, err);
    if (nread != sz) {
        std::ostringstream msg;
        msg << "can't read memory at address ";
        util::print_hex(msg, offset, sizeof(offset), 4);

        if (err.Fail()) {
            auto errstr = err.GetCString();
            if (errstr) {
                msg << ": " << errstr;
            }
        }

        throw dbgfmt::data::error{msg.str()};
    }
}


void lldb_memory_storage::write(const void * data, size_t sz, uint64_t offset) {
    throw dbgfmt::data::error{"writing to lldb memory is not implemented"};
}

}
