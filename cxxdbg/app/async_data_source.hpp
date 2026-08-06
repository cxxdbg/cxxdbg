// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file async_data_source.hpp
/// Contains definition of the async_data_source class.

#pragma once

#include "signals.hpp"
#include <cstdint>
#include <functional>
#include <vector>


namespace cxxdbg {


/// State of async data source
enum class async_data_source_sate {
    /// Data is unavailable. Clients should not try read data and should display
    /// empty representation of data.
    unavailable,

    /// Data is available. Clients can read data normally.
    available,
    
    /// Data is now being updated. Clients should not try read data while it is
    /// being updated, and should reload all data after state becomes available.
    updating
};


/// Abstract data source that can obtain binary data asynchronously.
class async_data_source {
public:
    /// State of data source
    using state_t = async_data_source_sate;

    /// Type of data read completion handler
    using completion_handler = std::function<void (const std::vector<char> &)>;

    /// Virtual destructor
    virtual ~async_data_source() = default;

    /// Returns data size
    virtual uint64_t size() const = 0;

    /// Returns data source state
    virtual state_t state() const = 0;

    /// Reads data asynchronously and invokes completion handler when data is read.
    /// If read_data is called multiple times before completion handler is invoked,
    /// then it's guaranteed that completion handlers will be invoked in the same order
    /// as calls to read_data.
    virtual void read_data(uint64_t offset, size_t size, const completion_handler & handler) const = 0;

    /// The signal is emitted after data source state was changed
    signal<void ()> state_changed;
};


}
