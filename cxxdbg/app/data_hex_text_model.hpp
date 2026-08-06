// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file data_hex_text_model.hpp
/// Contains definition of the data_hex_text_model class.

#pragma once

#include "tmvc/simple_text_model.hpp"
#include "tmvc/signals.hpp"
#include "vector_model.hpp"


namespace cxxdbg {


/// Text model that displays binary data as text in hexadecimal form.
class data_hex_text_model {
public:
    /// Type of model for binary data
    using binary_data_model = vector_model<char>;

    /// Constructs model with specified reference to binary data model and data
    /// representation parameters
    data_hex_text_model(const binary_data_model & data,
                        size_t column_byte_size = 8,
                        size_t columns_count = 2);

    /// Returns reference to text model that represents data in hexadecimal form
    const tmvc::wsimple_text_model & text() const { return text_; }

    /// Returns number of bytes in column
    auto column_byte_size() const { return column_byte_size_; }

    /// Returns number of columns
    auto columns_count() const { return columns_count_; }

    /// Returns number of bytes in row
    size_t row_byte_size() const { return column_byte_size() * columns_count(); }

private:
    /// Called when new data is added
    void on_data_added(size_t first_idx, size_t last_idx);

    /// Called when data is changed
    void on_data_changed(size_t idx);


    const binary_data_model & data_;            ///< Reference to binary data model
    tmvc::wsimple_text_model text_;             ///< Text model

    const size_t column_byte_size_;     ///< Number of bytes to display in each column
    const size_t columns_count_;        ///< Number of columns to displays

    /// Connection to data added signal
    tmvc::scoped_signal_connection data_added_con_;

    /// Connection to data changed signal
    tmvc::scoped_signal_connection data_changed_con_;
};



}
