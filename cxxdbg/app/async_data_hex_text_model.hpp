// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file async_data_hex_text_model.hpp
/// Contains definition of the async_data_hex_text_model class.

#pragma once

#include "async_data_fetcher.hpp"
#include "async_data_source.hpp"
#include "data_hex_text_model.hpp"
#include "editor_model.hpp"


namespace cxxdbg {


/// Text model that represents binary data from an asynchronous source
/// as text in hexadecimal format
class async_data_hex_text_model {
public:
    /// Constructs text model with specified reference to async data source
    /// and text representation parameters
    async_data_hex_text_model(async_data_source & src,
                              size_t column_byte_size = 8,
                              size_t columns_count = 2):
    data_fetcher_{src},
    hex_mdl_{data_fetcher_.data(), column_byte_size, columns_count} {
    }

    /// Default destructor
    ~async_data_hex_text_model() = default;

    /// Returns reference to text model that contains hex representation of data
    const wdocument_model & text() const { return hex_mdl_.text(); }

    /// Resest data view to new position and fetches specified number of lines
    void reset(uint64_t addr, size_t lines_count) {
        data_fetcher_.reset(addr, lines_count * hex_mdl_.row_byte_size());
    }

    /// Fetches data before beginning of currently fetched data to fill specified number
    /// of lines in text representation
    void fetch_begin(size_t lines_count) {
        data_fetcher_.fetch_begin(lines_count * hex_mdl_.row_byte_size());
    }

    /// Fetches data after the end of currently fetched data to fill specified number
    /// of lines in text representation
    void fetch_end(size_t lines_count) {
        data_fetcher_.fetch_end(lines_count * hex_mdl_.row_byte_size());
    }

private:
    async_data_fetcher data_fetcher_;       ///< Fetched of data from async data source
    data_hex_text_model hex_mdl_;           ///< Hex representation of fetched data
};


}
