// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file simple_tree_view_model.cpp
/// Contains implementation of the simple_tree_view_model class.

#include "simple_tree_view_model.hpp"


namespace cxxdbg {


size_t simple_tree_view_model::node::child_index(simple_tree_view_model::node * chld) const {
    auto res = std::find_if(childs_.begin(), childs_.end(), [chld](const auto & n) {
        return n.get() == chld;
    });

    assert(res != childs_.end() && "can't find child node in parent");
    return std::distance(childs_.begin(), res);
}


std::size_t simple_tree_view_model::childs_size(const row_index & row) const {
    return row_node(row)->childs_size();
}


auto simple_tree_view_model::child(const row_index & row, std::size_t index) const -> row_index {
    return make_index(row_node(row)->child(index).get());
}


void simple_tree_view_model::insert(const ro_tree_view_model::row_index & row,
                                    size_t idx,
                                    const std::vector<std::vector<std::wstring>> & vals) {
    auto num_nodes = vals.size();

    // notifying observers about adding new nodes
    before_added()(row, idx, idx + num_nodes - 1);

    node * p = row_node(row);

    // adding nodes into parent
    for (size_t i = 0; i < num_nodes; ++i) {
        // creating new node with specified values
        auto n = std::make_shared<node>(p, column_names_.size());
        for (size_t ncol = 0, ncol_e = vals[i].size(); ncol < ncol_e; ++ncol) {
            n->set_text(ncol, vals[i][ncol]);
        }

        p->insert(idx + i, n);
    }

    // sending after_added signal to observers
    after_added()(row, idx, idx + num_nodes - 1);
}


void simple_tree_view_model::remove(const ro_tree_view_model::row_index & row,
                                    size_t idx,
                                    size_t nchilds) {
    auto end_idx = idx + nchilds - 1;
    assert(end_idx < childs_size({}) && "invalid childs indexes");

    before_removed()(row, idx, end_idx);
    row_node(row)->remove(idx, nchilds);
    after_removed()(row, idx, end_idx);
}


std::size_t simple_tree_view_model::index(const row_index & row) const {
    assert(row.is_valid() && "invalid row index");
    node * n = row_node(row);
    node * pnode = n->parent();
    return pnode->child_index(n);
}


auto simple_tree_view_model::parent(const row_index & row) const -> row_index {
    assert(row.is_valid() && "invalid row index");
    node * pnode = row_node(row)->parent();

    // row index of root node is invalid
    if (pnode == &root_) {
        return {};
    }

    return make_index(pnode);
}


std::wstring simple_tree_view_model::text(const row_index & row, std::size_t c) const {
    return row_node(row)->text(c);
}


bool simple_tree_view_model::editable(const ro_tree_view_model::row_index & r, std::size_t c) {
    return editable_;
}


void simple_tree_view_model::set_text(const ro_tree_view_model::row_index & r,
                                      std::size_t c,
                                      const std::wstring & s) {
    row_node(r)->set_text(c, s);
    after_changed()(r);
}


}
