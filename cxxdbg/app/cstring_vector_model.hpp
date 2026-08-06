// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file cstring_vector_model.hpp
/// Contains definition of the cstring_vector_model class.

#pragma once

#include "cxxdbg/app/tree_view_model.hpp"


namespace cxxdbg {


/// Tree view model for vector of C strings
class cstring_vector_model: public ro_tree_view_model {
public:
    /// Type of vector of source files
    typedef std::vector<const char *> cstring_vector;

    /// Constructor, makes model with specified pointer to vector
    /// of C strings
    cstring_vector_model(const cstring_vector * strings = nullptr);

    /// Sets pointer to vector of C string
    void set_strings(const cstring_vector * strings);

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

private:
    const cstring_vector * strings_;        ///< Pointer to vector of C strings
};


}


