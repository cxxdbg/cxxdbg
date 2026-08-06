// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file cstring_vector_model.cpp
/// Contains implementation of the cstring_vector_model class.

#include "cstring_vector_model.hpp"


namespace cxxdbg {


cstring_vector_model::cstring_vector_model(const cstring_vector * strings):
strings_{nullptr} {
    set_strings(strings);
}


void cstring_vector_model::set_strings(const cstring_vector * strings) {
    if (strings_ != nullptr) {
        auto sz = strings_->size();

        if (sz != 0) {
            before_removed()({}, 0, sz - 1);
        }

        strings_ = nullptr;

        if (sz != 0) {
            after_removed()({}, 0, sz - 1);
        }
    }

    if (strings != nullptr) {
        auto sz = strings->size();

        // sending add signals
        if (sz != 0) {
            before_added()({}, 0, sz - 1);
        }

        strings_ = strings;

        if (sz != 0) {
            after_added()({}, 0, sz - 1);
        }
    }
}


std::size_t cstring_vector_model::columns_size() const {
    return 1;
}


std::wstring cstring_vector_model::column_name(std::size_t index) const {
    return {};
}


std::size_t cstring_vector_model::childs_size(const row_index & row) const {
    if (row) {
        // no childs in rows
        return 0;
    }

    if (strings_ == nullptr)
        return 0;

    return strings_->size();
}


cstring_vector_model::row_index
cstring_vector_model::child(const row_index & row, std::size_t index) const {
    assert(!row && "no childs in rows");
    assert(strings_ != nullptr && "strings should not be null");
    assert(index < strings_->size() && "invalid index");
    return make_index(static_cast<uintptr_t>(index));
}


std::size_t cstring_vector_model::index(const row_index & row) const {
    assert(row && "invaldi row index");
    return static_cast<size_t>(row.val());
}


cstring_vector_model::row_index cstring_vector_model::parent(const row_index & row) const {
    return {};
}


std::wstring cstring_vector_model::text(const row_index & row, std::size_t c) const {
    assert(row && "invalid row index");
    assert(c == 0 && "invalid column index");

    size_t idx = index(row);
    assert(strings_ != nullptr && "strings should not be null");
    assert(idx < strings_->size() && "invalid index");

    const char * str = (*strings_)[idx];
    const char * end = str;
    while (*end != 0) {
        ++end;
    }

    return {str, end};
}


}
