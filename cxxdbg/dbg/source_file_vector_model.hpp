// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file source_file_vector_model.hpp
/// Contains definition of the source_file_vector_model class.

#pragma once

#include "cxxdbg/app/tree_view_model.hpp"
#include <vector>


namespace cxxdbg::dbg {


class source_file;


/// Tree view model for vector of source files which displays
/// source names
class source_file_vector_model: public ro_tree_view_model {
public:
    /// Type of vector of source files
    typedef std::vector<const source_file*> source_file_vector;

    /// Constructor, makes model with specified pointer to vector
    /// of source files
    source_file_vector_model(const source_file_vector * sources = nullptr);

    /// Sets pointer to vector of source files
    void set_sources(const source_file_vector * sources);

    /// Returns number of columns
    std::size_t columns_size() const override;

    /// Returns name of column with specified index
    std::wstring column_name(std::size_t index) const override;

    /// Returns number of child rows for row
    std::size_t childs_size(const row_index & row) const override;

    /// Returns child row with specified index
    row_index child(const row_index & row, std::size_t index) const override;

    /// Returns index of row in parent
    std::size_t index(const row_index & row) const override;

    /// Returns parent for specified row
    row_index parent(const row_index & row) const override;

    /// Returns text for specified cell
    std::wstring text(const row_index & row, std::size_t c) const override;

    /// Returns image index for specified cell. Default implementation returns 0.
    image_index image(const row_index & row, std::size_t c) const override;

private:
    const source_file_vector * sources_;    ///< Pointer to vector of sources
};


}


