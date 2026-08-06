// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file async_data_fetcher.hpp
/// Contains definition of the async_data_fetcher class.

#pragma once

#include "async_data_source.hpp"
#include "vector_model.hpp"


namespace cxxdbg {


/// Helper class that fetches data from async data source and fills vector model
/// containing binary data.
class async_data_fetcher {
public:
    /// Constructs data fetcher with specified reference to data source
    async_data_fetcher(async_data_source & src);

    /// Destroys fetcher. Checks that all operations were completed.
    ~async_data_fetcher();

    /// Returns const reference to model containing fetched data
    const vector_model<char> & data() const { return data_; }

    /// Resets start addres. Clears current fetched region and starts fetching specified
    /// amount of bytes, if not zero.
    void reset(uint64_t s_addr, size_t size);

    /// Starts fetching specified amount of bytes before current fetched region from data source
    void fetch_begin(size_t size);

    /// Starts fetching specified amount of bytes after current fetched region from data source
    void fetch_end(size_t size);

    /// Returns current start address
    uint64_t start_addr() const { return start_addr_; }

private:
    async_data_source & data_src_;              ///< Reference to current data source
    vector_model<char> data_;                   ///< Fetched data
    uint64_t start_addr_;                       ///< Start address for fetched data

    unsigned int n_reset_ops_ = 0;              ///< Number of reset operations in progress
    unsigned int n_fetch_begin_ops_ = 0;        ///< NUmber of fetch begin operations in progress
    unsigned int n_fetch_end_ops_ = 0;          ///< Number of fetch end operations in progress
    unsigned int n_skip_fetch_begin_ops_ = 0;   ///< Number of fetch beign operations to skip
    unsigned int n_skip_fetch_end_ops_ = 0;     ///< Number of fetch end operations to skip
    size_t fetch_begin_size_ = 0;               ///< Size of data being fetched at the beginning
    size_t fetch_end_size_ = 0;                 ///< Size of data being fetched at the end
};


}
