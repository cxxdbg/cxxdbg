// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file lldb_value_storage.cpp
/// Contains implementation of the lldb_value_storage class.

#include "lldb_value_storage.hpp"
#include "dbgfmt/data/error.hpp"
#include "cxxdbg/util/print.hpp"
#include <lldb/API/SBError.h>


namespace cxxdbg::dbg::core {


void lldb_value_storage::read(void * data, size_t sz, uint64_t offset) {
    auto val_data = val_.GetData();
    lldb::SBError err;
    val_data.ReadRawData(err, offset, data, sz);

    if (err.Fail()) {
        auto err_str = err.GetCString();
        if (err_str) {
            throw dbgfmt::data::error(err_str);
        } else {
            std::ostringstream msg;
            msg << "unknown error while reading data from lldb value at offset ";
            util::print_hex(msg, offset, sizeof(offset), sizeof(offset));
            throw dbgfmt::data::error{msg.str()};
        }
    }
}


void lldb_value_storage::write(const void * data, size_t sz, uint64_t offset) {
    throw dbgfmt::data::error{"writing to lldb value is not implemented"};
}


}
