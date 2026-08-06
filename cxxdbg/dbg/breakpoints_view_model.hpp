// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

//
// Created by yzarudny on 09.10.17.
//

#pragma once

#include "breakpoint.hpp"
#include "breakpoint_location.hpp"
#include "breakpoints_view_tree_node.hpp"
#include "cxxdbg/app/tree_view_model.hpp"


namespace cxxdbg::dbg {


/// @class breakpoints_view_model implements model for breakpoints view
class breakpoints_view_model: public ro_tree_view_model {
public:
    using tree_node = breakpoints_view_tree_node;

    /// Constructor
    explicit breakpoints_view_model(breakpoint_list & bplist);

    /// Destructor
    ~breakpoints_view_model() override = default;

    /// Returns number of columns
    std::size_t columns_size() const override;

    /// Returns name of column with specified index
    std::wstring column_name(std::size_t index) const override ;

    /// Returns number of child rows for row
    std::size_t childs_size(const row_index & row) const override ;

    /// Returns child row with specified index
    row_index child(const row_index & row, std::size_t index) const override ;

    /// Returns index of row in parent
    std::size_t index(const row_index & row) const override;

    /// Returns parent for specified row
    row_index parent(const row_index & row) const override;

    /// Returns text for specified cell
    std::wstring text(const row_index & row, std::size_t c) const override;

    /// Returns image index for specified cell. Default implementation returns 0.
    image_index image(const row_index & row, std::size_t c) const override;

    /// Returns color index for specified cell. Default implementation returns 0.
    color_index color(const row_index & row, std::size_t c) const override;

    /// Returns essential location for index
    const breakpoint_location * get_single_location(const row_index & row) const;

    /// Returns breakpoint_site for specified index
    const breakpoint_site * get_breakpoint_site(const row_index & row) const;

    /// Tries to convert index to code_breakpoint
    /// @return code_breakpoint pointer or nullptr if cannot be converted
    const breakpoint * get_breakpoint(const row_index & row) const;

    /// Breakpoint signals signature
    typedef void breakpoint_function(const breakpoint *);

    /// Called when new breakpoint added
    void on_breakpoint_added(const breakpoint * bp);

    /// Called when breakpoint updated
    void on_breakpoint_updated(const breakpoint * bp);

    /// Called when breakpoint removed
    void on_breakpoint_removed(const breakpoint * bp);

    /// Converts internal pointer of an index to tree_node *
    static tree_node * make_tree_node(const row_index & row);

    /// Makes index from tree node
    static row_index make_index(tree_node * node);

private:
    breakpoint_list & bplist_;              ///< Reference to list of breakpoints
    tree_node::node_vector root_nodes_;     ///< Root nodes vector
    tree_node::node_map root_map_;          ///< Map bp -> tree node

    /// Breakpoint added connection holder
    scoped_signal_connection breakpoint_added_connection_;

    /// Breakpoint removed connection holder
    scoped_signal_connection breakpoint_removed_connection_;

    /// Breakpoint changed connection holder
    scoped_signal_connection breakpoint_changed_connection_;
};

}

