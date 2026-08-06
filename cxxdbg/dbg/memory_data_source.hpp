// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file memory_data_source.hpp
/// Contains definition of the memory_data_source class.

#pragma once

#include "cxxdbg/app/async_data_source.hpp"


namespace cxxdbg::dbg {

class target;


/// Data source implementation that represents memory of process being debugged
class memory_data_source: public async_data_source {
public:
    /// Constructs memory data source for specified target
    memory_data_source(target & targ): targ_{targ} {}

    /// Destroys data source
    ~memory_data_source() override = default;

    /// Returns data size
    uint64_t size() const override { return UINT64_MAX; }

    /// Reads data asynchronously and invokes completion handler when data is read.
    /// If read_data is called multiple times before completion handler is invoked,
    /// then it's guaranteed that completion handlers will be invoked in the same order
    /// as calls to read_data.
    void read_data(uint64_t offset, size_t size, const completion_handler & handler) const override;

private:
    target & targ_;             ///< Reference to target
};


}
