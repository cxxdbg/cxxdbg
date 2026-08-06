// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <map>

namespace cxxdbg::dbg {

class breakpoint;
class breakpoint_site;
class code_breakpoint;
class breakpoint_location;
class watchpoint;

/// Class for breakpoint tree node
class breakpoints_view_tree_node {
public:
    using tree_node = breakpoints_view_tree_node;
    using tree_node_ptr = std::shared_ptr<tree_node> ;
    using node_vector = std::vector<tree_node_ptr>;
    using node_map = std::map<const breakpoint *, tree_node *>;

    class node_info;
    using info_ptr = std::shared_ptr<node_info>;

    enum node_type {type_code_breakpoint, type_watchpoint, type_location};

    /// Makes new node
    static tree_node_ptr create_node(const breakpoint * bp);

    /// Creates node location
    tree_node_ptr add_location(const breakpoint_location * loc);

    /// Constructor, creates node in general form
    explicit breakpoints_view_tree_node(info_ptr info, tree_node * parent = nullptr);

    /// Returns number of children
    std::size_t children_size() const { return children_.size(); }

    /// Provides access to child at i-th position
    tree_node * child_at(size_t i) const;

    /// Returns node parent
    tree_node * parent() const { return parent_; }

    /// Node parent index
    std::size_t parent_index() const;

    /// Returns node info - data part of the node
    node_info * get_info() const;

    /// Removes all child nodes
    void remove_all_children();

    /// Adds new child node to the end of children vector
    void add_child(tree_node_ptr node);

private:
    /// Creates breakpoint type node
    static tree_node_ptr make_breakpoint_node(const code_breakpoint * bp);

    /// Creates watchpoint type node
    static tree_node_ptr make_watchpoint_node(const watchpoint * wps);

    std::shared_ptr<node_info> info_;       ///< Info - data part of the node
    breakpoints_view_tree_node * parent_;   ///< Node parent
    node_vector children_;                  ///< Child nodes
};

/// Class breakpoints_view_tree_node::node_info
/// Interface for data part of tree node
class breakpoints_view_tree_node::node_info {
public:
    /// Destructor
    virtual ~node_info() {}
    /// @return node name
    virtual std::wstring name() const = 0;
    /// @return condition value
    virtual std::wstring condition() const = 0;
    /// @return hit count value
    virtual std::wstring hit_count() const = 0;
    /// @return function value
    virtual std::wstring function() const = 0;
    /// @return address value
    virtual std::wstring address() const = 0;
    /// @return icon index
    virtual std::size_t image_index() const = 0;
    /// @return breakpoint site
    virtual const breakpoint_site * get_site() const = 0;
    /// @return node type
    virtual node_type type() const = 0;
};

}

