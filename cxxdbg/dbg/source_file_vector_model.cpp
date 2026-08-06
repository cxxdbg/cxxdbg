// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file source_file_vector_model.cpp
/// Contains implementation of the source_file_vector_model class.

#include "source_file_vector_model.hpp"
#include "source_file.hpp"


namespace cxxdbg::dbg {


source_file_vector_model::source_file_vector_model(const source_file_vector * sources):
sources_{nullptr} {
    set_sources(sources);
}


void source_file_vector_model::set_sources(const source_file_vector * sources) {

    if (sources_ != nullptr) {
        auto sz = sources_->size();

        if (sz != 0) {
            before_removed()({}, 0, sz - 1);
        }

        sources_ = nullptr;

        if (sz != 0) {
            after_removed()({}, 0, sz - 1);
        }
    }

    if (sources != nullptr) {
        auto sz = sources->size();

        // sending add signals
        if (sz != 0) {
            before_added()({}, 0, sz - 1);
        }

        sources_ = sources;

        if (sz != 0) {
            after_added()({}, 0, sz - 1);
        }
    }
}


std::size_t source_file_vector_model::columns_size() const {
    return 1;
}


std::wstring source_file_vector_model::column_name(std::size_t index) const {
    return {};
}


std::size_t source_file_vector_model::childs_size(const row_index & row) const {
    if (row) {
        // no childs in rows
        return 0;
    }

    if (sources_ == nullptr)
        return 0;

    return sources_->size();
}


source_file_vector_model::row_index
source_file_vector_model::child(const row_index & row, std::size_t index) const {
    assert(!row && "no childs in rows");
    assert(sources_ != nullptr && "sources should not be null");
    assert(index < sources_->size() && "invalid index");
    return make_index(static_cast<uintptr_t>(index));
}


std::size_t source_file_vector_model::index(const row_index & row) const {
    assert(row && "invalid row index");
    return static_cast<size_t>(row.val());
}


source_file_vector_model::row_index source_file_vector_model::parent(const row_index & row) const {
    return {};
}


std::wstring source_file_vector_model::text(const row_index & row, std::size_t c) const {
    assert(row && "invalid row index");
    assert(c == 0 && "invalid column index");

    size_t idx = index(row);
    assert(sources_ != nullptr && "sources should not be null");
    assert(idx < sources_->size() && "invalid index");
    return (*sources_)[idx]->path().filename().wstring();
}


source_file_vector_model::image_index source_file_vector_model::image(const row_index & row, std::size_t c) const {
    return 0;
}


}
