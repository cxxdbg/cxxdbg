// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file watch_model.hpp
/// Contains definition of the watch_model class.

#pragma once

#include "cxxdbg/app/tree_view_model.hpp"


namespace cxxdbg::dbg {


class watch_list;
class watch_list_tree_node;


/// Tree view model for watch list
class watch_model: virtual public ro_tree_view_model {
public:
    /// Constructor, makes watch list model for specified
    /// watch list
    watch_model(watch_list & wlist);

    /// Destructor, destoys object
    virtual ~watch_model();

    /// Returns number of columns
    std::size_t columns_size() const override;

    /// Returns name of column with specified index
    std::wstring column_name(std::size_t index) const override;

    /// Returns number of child rows for row
    std::size_t childs_size(const row_index & row = {}) const override;

    /// Returns child row with specified index
    row_index child(const row_index & row, std::size_t index) const override;

    /// Returns index of row in parent
    std::size_t index(const row_index & row) const override;

    /// Returns parent for specified row
    row_index parent(const row_index & r) const override;

    /// Returns text for specified cell
    std::wstring text(const row_index & r, std::size_t c) const override;

    /// Returns color index for specified cell. Default implementation returns 0.
    color_index color(const row_index & row, std::size_t c) const override;

    /// Returns true if model has expand_state interface support and false otherwise, no support by default
    bool has_expand_state_support() const override { return true; }

    /// Returns expand state for selected index, undefined by default
    tree_view_expand_state get_expand_state(const row_index & index) const override;

    /// Sets expand state
    void set_expand_state(const row_index & index, tree_view_expand_state state) const override;

protected:
    /// Returns watch node for row with specified index
    const watch_list_tree_node * row_node(const row_index & row) const;

    /// Returns row index for watch node
    row_index make_node_index(const watch_list_tree_node * node) const;

private:
    /// Returns index of node in parent node
    std::size_t index_in_parent(const watch_list_tree_node * node) const;

    /// Called after root node added in watch list
    virtual void on_root_node_added(const watch_list_tree_node * node);

    /// Called before root node removed from watch list
    virtual void on_root_node_removed(const watch_list_tree_node * node);

    /// Called when node changed in watch list
    void on_node_changed(const watch_list_tree_node * node);


    watch_list & watch_;        ///< Reference to watch list

    /// Node for which we are updating childs now
    const watch_list_tree_node * update_childs_node_{nullptr};

    /// Number of childs in node being updated
    std::size_t update_childs_node_size_{0};

    /// Vector of root nodes. Watch list can change order of root nodes,
    /// but tree model should preserve order.
    std::vector<const watch_list_tree_node *> root_nodes_;


    // signal connections
    scoped_signal_connection root_node_added_con_;
    scoped_signal_connection root_node_removed_con_;
    scoped_signal_connection node_changed_con_;

    scoped_signal_connection before_added_con_;
    scoped_signal_connection after_added_con_;
    scoped_signal_connection before_removed_con_;
    scoped_signal_connection after_removed_con_;

    scoped_signal_connection node_visually_expand_con_;
};


}


