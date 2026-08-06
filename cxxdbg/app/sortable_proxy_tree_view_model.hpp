// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file sortable_proxy_tree_view_model.hpp
/// Contains definitions of the sorted model classes.

#pragma once

#include "tree_view_model.hpp"
#include <unordered_map>


namespace cxxdbg {


/// Read only proxy model that supports sorting
class sortable_proxy_ro_tree_view_model:
        virtual public sortable_ro_tree_view_model {
public:
    /// Constructs proxy model with specified reference to source model
    sortable_proxy_ro_tree_view_model(ro_tree_view_model & src_model);

    /// Destroys object
    virtual ~sortable_proxy_ro_tree_view_model() = default;

    /// Returns number of columns. Returns same value as
    /// source model columns_size reutrns.
    std::size_t columns_size() const override;

    /// Returns name of column with specified index. Returns
    /// same value as source model column_name returns.
    std::wstring column_name(std::size_t index) const override;

    /// Returns number of child rows for row. Returns same value as
    /// source model childs_size returns for translated row index.
    std::size_t childs_size(const row_index & row) const override;

    /// Returns child row with specified index.
    row_index child(const row_index & row, std::size_t index) const override;

    /// Returns index of row in parent
    std::size_t index(const row_index & row) const override;

    /// Returns parent for specified row
    row_index parent(const row_index & row) const override;

    /// Returns text for specified cell
    std::wstring text(const row_index & row, std::size_t c) const override;

    /// Returns image index for specified cell. Delegates call to
    /// source model.
    image_index image(const row_index & row, std::size_t c) const override;

    /// Returns color index for specified cell. Delegates call to source model.
    color_index color(const row_index & row, std::size_t c) const override;

    /// Sets sort order for model
    void sort(size_t column, sort_order order) override;

    /// Returns true if model has expand_state interface support and false otherwise, no support by default
    bool has_expand_state_support() const override;

    /// Returns expand state for selected index, undefined by default
    tree_view_expand_state get_expand_state(const tree_view_model_row_index & index) const override;

    /// Sets expand state for selected index
    void set_expand_state(const row_index & index, tree_view_expand_state state) const override;

private:
    /// Called before items added in source model
    void on_before_added(const row_index & idx, size_t start, size_t end);

    /// Called after items added in source model
    void on_after_added(const row_index & idx, size_t start, size_t end);

    /// Called before items removed from source model
    void on_before_removed(const row_index & idx, size_t start, size_t end);

    /// Called after items removed from source model
    void on_after_removed(const row_index & idx, size_t start, size_t end);

    /// Called after item changed in source model
    void on_after_changed(const row_index & idx);

    /// Called before layout of child items changed in source model
    void on_before_layout_changed(const row_index & idx);

    /// Called after layout of child items has been changed in source model
    void on_after_layout_changed(const row_index & idx);

    /// Calculates insert index of new root node taking into account
    /// current sorting order
    size_t calc_insert_index(const row_index & idx);

    /// Inserts root node into vector of nodes taking into account
    /// current sorting order
    void insert_root_node(const row_index & idx);


    ro_tree_view_model & src_mdl_;          ///< Reference to source model
    size_t sort_col_ = 0;                   ///< Current sort column
    sort_order order_ = sort_order::no;     ///< Current sort order
    std::vector<row_index> nodes_;          ///< Sorted vector of root indexes

    scoped_signal_connection before_added_con_;
    scoped_signal_connection after_added_con_;
    scoped_signal_connection before_removed_con_;
    scoped_signal_connection after_removed_con_;
    scoped_signal_connection after_changed_con_;
    scoped_signal_connection before_layout_changed_con_;
    scoped_signal_connection after_layout_changed_con_;
};


/// Writable proxy model that supports sorting
class sortable_proxy_tree_view_model:
        public sortable_proxy_ro_tree_view_model,
        virtual public sortable_tree_view_model {

public:
    /// Constructs sorted model with specified reference to source model
    sortable_proxy_tree_view_model(tree_view_model & src_mdl);

    /// Destroys object
    virtual ~sortable_proxy_tree_view_model() = default;

    /// Returns true if cell is editable
    bool editable(const row_index & r, std::size_t c) override;

    /// Sets text for specified cell
    void set_text(const row_index & r, std::size_t c, const std::wstring & s) override;

private:
    tree_view_model & src_mdl_;     ///< Reference to source model
};


}


