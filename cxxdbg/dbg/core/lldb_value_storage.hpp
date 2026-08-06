// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file lldb_value_storage.hpp
/// Contains definition of the lldb_value_storage class.

#pragma once

#include "dbgfmt/data/storage.hpp"
#include <lldb/API/SBValue.h>


namespace cxxdbg::dbg::core {


/// Implementation of data storage that reads/write to/from lldb value
class lldb_value_storage: public dbgfmt::data::storage {
public:
    /// Constructs data storage for specified lldb value
    lldb_value_storage(const lldb::SBValue & v):
        val_{v} {}

    /// Reads bytes from lldb value at specified offset.
    /// Throws error if data can't be read.
    void read(void * data, size_t sz, uint64_t offset) override;

    /// Writes bytes to lldb value at specified offset.
    /// Throws error if data can't be written
    void write(const void * data, size_t sz, uint64_t offset) override;

private:
    lldb::SBValue val_;
};


}
