// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file composite_source_tree_model.hpp
/// Contains definition of the composite_source_tree_model class.

#pragma once


#include "cxxdbg/app/tree_view_model.hpp"
#include <cassert>
#include <filesystem>
#include <set>


namespace cxxdbg::dbg {


class source_tree;
class source_tree_item;


/// Tree view model for source tree that displays chains of single nested directories
/// as single item
class composite_source_tree_model: public ro_tree_view_model {
public:
    static constexpr image_index image_index_file = 1;
    static constexpr image_index image_index_directory = 2;

    /// Constructs model with specified reference to source tree
    composite_source_tree_model(source_tree & src);

    /// Returns number of columns
    size_t columns_size() const override { return 1; }

    /// Returns name of column with specified index
    std::wstring column_name(std::size_t index) const override {
        assert(index == 0 && "invalid column index");
        return {};
    }

    /// Returns number of child rows for row
    size_t childs_size(const row_index & row) const override;

    /// Returns child row with specified index
    row_index child(const row_index & row, std::size_t index) const override;

    /// Returns index of row in parent
    size_t index(const row_index & row) const override;

    /// Returns parent for specified row
    row_index parent(const row_index & row) const override;

    /// Returns text for specified cell
    std::wstring text(const row_index & row, std::size_t c) const override;

    /// Returns image index for specified cell. Default implementation returns 0.
    image_index image(const row_index & row, std::size_t c) const override;

    /// Returns true if model has expand_state interface support. Always return true.
    bool has_expand_state_support() const override { return true; }

    /// Returns expand state of specified index
    tree_view_expand_state get_expand_state(const row_index & index) const override;

    /// Sets expand state for specified index
    void set_expand_state(const row_index & index, tree_view_expand_state state) const override;

    /// Returns index of item with specified path
    row_index path_index(const std::filesystem::path & p) const;

    /// Returns tree node by row index
    const source_tree_item * row_to_node(const row_index & idx) const { return tree_node(idx); }

private:
    /// Returns tree node by row index
    const source_tree_item * tree_node(const row_index & idx) const;

    /// Makes row index by tree node
    row_index make_index(const source_tree_item * item) const;

    /// Saves all expanded childs nodes of node to removed_node_expanded_childs_ set
    void save_removed_expanded_childs(const source_tree_item * node);

    source_tree & sources_;         ///< Reference to source tree

    size_t removed_node_index_ = SIZE_MAX;                  ///< Index of emulated removed node 
    const source_tree_item * removed_node_parent_ = nullptr;///< Parent of emulated removed node
    const source_tree_item * removed_node_ = nullptr;       ///< Emulated removed node

    /// Set of expanded childs in emulated removed node
    std::set<const source_tree_item *> removed_node_expanded_childs_;

    /// Set of expanded tree nodes
    mutable std::set<const source_tree_item *> expanded_nodes_;
};


}
