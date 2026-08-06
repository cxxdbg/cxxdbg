// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file lldb_memory_storage.hpp
/// Contains definition of the lldb_memory_storage class.

#pragma once

#include "dbgfmt/data/storage.hpp"
#include <lldb/API/SBTarget.h>


namespace cxxdbg::dbg::core {


/// Data storage implementation for lldb target memory
class lldb_memory_storage: public dbgfmt::data::storage {
public:
    /// Constructs memory storage for specified lldb target
    lldb_memory_storage(const lldb::SBTarget & t):
        targ_{t} {}

    /// Reads bytes from lldb targets memory at specified offset.
    /// Throws error if data can't be read.
    void read(void * data, size_t sz, uint64_t offset) override;

    /// Writes bytes to lldb targets memory at specified offset
    void write(const void * data, size_t sz, uint64_t offset) override;

private:
    lldb::SBTarget targ_;           ///< LLDB target to read/write memory from
};


}
