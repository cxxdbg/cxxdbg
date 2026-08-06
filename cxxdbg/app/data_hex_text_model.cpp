// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file data_hex_text_model.cpp
/// Contains implementation of the data_hex_text_model class.

#include "data_hex_text_model.hpp"
#include "cxxdbg/util/print.hpp"


namespace cxxdbg {


/// Writes hexadecimal bytes to output stream in multiple rows
template <typename Iterator>
static void write_hex_rows(std::wostringstream & str,
                           size_t column_size,
                           size_t columns_count,
                           size_t start_curr_columns_count,
                           size_t start_curr_column_size,
                           Iterator first,
                           Iterator last) {

    size_t curr_columns_count = start_curr_columns_count;
    size_t curr_column_size = start_curr_column_size;
    bool is_first = true;

    for (auto it = first; it != last; ++it) {
        // writing space separator between hex numbers and columns
        if (curr_column_size == 0) {
            if (curr_columns_count != 0) {
                // beginning of new column
                str << L"    ";
            } else {
                // beginning of row
                if (!is_first) {
                    str << L'\n';
                }
            }
        } else {
            // new byte in same column
            str << L' ';
        }

        is_first = false;

        // writing hex number
        util::print_hex_no_prefix(str, static_cast<unsigned char>(*it), 1, 1);

        // checking for the end of current column
        ++curr_column_size;
        if (curr_column_size == column_size) {
            curr_column_size = 0;
            ++curr_columns_count;
        }

        // checking for the end of current row
        if (curr_columns_count == columns_count) {
            curr_columns_count = 0;
        }
    }
}


/// Writes hexadecimal bytes to string in multiple rows
template <typename Iterator>
static std::wstring write_hex_rows_to_string(size_t column_size,
                                             size_t columns_count,
                                             size_t start_curr_columns_count,
                                             size_t start_curr_column_size,
                                             Iterator first,
                                             Iterator last) {
    std::wostringstream str;
    write_hex_rows(str,
                   column_size,
                   columns_count,
                   start_curr_columns_count,
                   start_curr_column_size,
                   first,
                   last);
    return str.str();
}


data_hex_text_model::data_hex_text_model(const binary_data_model & data,
                                         size_t column_byte_size,
                                         size_t columns_count):
data_{data}, column_byte_size_{column_byte_size}, columns_count_{columns_count} {

    // listening for adding data at the beginning or at the end

    data_added_con_ = data_.after_added().connect([this](size_t first, size_t last) {
        on_data_added(first, last);
    });

    // listening for changing data
    data_changed_con_ = data_.changed().connect([this](size_t idx) {
        on_data_changed(idx);
    });

    // calling on_data added for initial data
    if (!data_.empty()) {
        on_data_added(0, data_.size() - 1);
    }
}


void data_hex_text_model::on_data_added(size_t first_idx, size_t last_idx) {
    assert(!data_.empty() && "data should not be empty after in added signal");

    if (first_idx == 0) {
        bool is_empty = characters(text_).empty();

        // Adding data at the beginning. The size of added data must be multiple to
        // number of bytes in row.
        auto data_size = last_idx - first_idx + 1;
        assert(is_empty || (data_size % row_byte_size() == 0) &&
               "data_hex_text_model supports adding only complete rows at the beginning");

        auto str = write_hex_rows_to_string(column_byte_size(),
                                            columns_count(),
                                            0,
                                            0,
                                            data_.begin() + first_idx,
                                            data_.begin() + last_idx + 1);

        if (!is_empty) {
            str.push_back(L'\n');
        }

        text_.insert(begin_pos(text_), str);

    } else if (last_idx == data_.size() - 1) {
        // adding data at the end

        size_t prev_data_size = first_idx;
        size_t last_row_size = prev_data_size % row_byte_size();
        size_t last_row_columns = last_row_size / column_byte_size();
        size_t last_row_pos_in_column = last_row_size % column_byte_size();

        auto str = write_hex_rows_to_string(column_byte_size(),
                                            columns_count(),
                                            last_row_columns,
                                            last_row_pos_in_column,
                                            data_.begin() + first_idx,
                                            data_.begin() + last_idx + 1);

        if (last_row_size == 0) {
            text_.insert(end_pos(text_), L"\n");
        }

        text_.insert(end_pos(text_), str);

    } else {
        assert(false && "adding data in the middle is not supported for data_hex_text_model");
    }
}


void data_hex_text_model::on_data_changed(size_t idx) {
    // calculating byte position in text view
    size_t row_idx = idx / row_byte_size();
    size_t idx_in_row = idx % row_byte_size();
    size_t column_idx = idx_in_row / column_byte_size();
    size_t idx_in_column = idx_in_row % column_byte_size();
    size_t pos_at_line = column_idx * ((2 + 1) * column_byte_size() + 3) +
                         idx_in_column * (2 + 1);

    // writing new hex number to stream
    std::wostringstream str;
    util::print_hex_no_prefix(str, data_.at(idx), 1, 1);

    // replacing hex number in text
    tmvc::position pos{row_idx, pos_at_line};
    tmvc::range del_range{pos, tmvc::position{row_idx, pos_at_line + 2}};
    text_.erase(del_range);
    text_.insert(pos, str.str());
}


}
