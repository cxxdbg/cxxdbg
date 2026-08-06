// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file tree_view_model.cpp
/// Contains implementation of helper classes for tree_view_model class.

#include "tree_view_model.hpp"
#include <cassert>


namespace cxxdbg {

        
// tree_view_model_row_index
tree_view_model_row_index::tree_view_model_row_index():
tree_view_model_row_index{nullptr} {
}


tree_view_model_row_index::~tree_view_model_row_index() {
}


void * tree_view_model_row_index::ptr() const {
    return ptr_;
}


uintptr_t tree_view_model_row_index::val() const {
    return reinterpret_cast<uintptr_t>(ptr()) - 1;
}


bool tree_view_model_row_index::is_valid() const {
    return ptr() != nullptr;
}


tree_view_model_row_index::operator bool() const {
    return is_valid();
}


bool tree_view_model_row_index::operator !() const {
    return !is_valid();
}


bool tree_view_model_row_index::operator==(const tree_view_model_row_index & r) const {
    return ptr() == r.ptr();
}


bool tree_view_model_row_index::operator!=(const tree_view_model_row_index & r) const {
    return !(*this == r);
}


tree_view_model_row_index::tree_view_model_row_index(void * ptr):
ptr_{ptr} {
}


tree_view_model_row_index::tree_view_model_row_index(uintptr_t v):
ptr_{reinterpret_cast<void*>(v + 1)} {
}



ro_tree_view_model::row_index ro_tree_view_model::make_index(void * ptr) {
    return row_index{ptr};
}


ro_tree_view_model::row_index ro_tree_view_model::make_index(uintptr_t val) {
    return row_index{val};
}


ro_tree_view_model::image_index
ro_tree_view_model::image(const row_index & row, std::size_t c) const {
    return 0;
}

ro_tree_view_model::color_index
ro_tree_view_model::color(const ro_tree_view_model::row_index & row, std::size_t c) const {
    return 0;
}


bool tree_view_model::editable(const row_index & r, std::size_t c) {
    return false;
}


void tree_view_model::set_text(const row_index & r, std::size_t c, const std::wstring & s) {
    assert(false && "editing model is not implemented");
}


std::string tree_view_expand_state_to_string(tree_view_expand_state st) {
    switch (st) {
    case tree_view_expand_state::expanded:
        return "expanded";
    case tree_view_expand_state::collapsed:
        return "collapsed";
    case tree_view_expand_state::undefined:
        return "undefined";
    default:
        assert(false && "unknown tree view expand state");
        return "unknown";
    }
}


}
