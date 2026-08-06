// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file watch_list_impl.hpp
/// Contains definition of watch_list_impl class.

#pragma once

#include <climits>
#include <vector>
#include <functional>

#include "source_position_info.hpp"
#include "cxxdbg/app/signals.hpp"
#include "cxxdbg/app/tree_view_model.hpp"
#include "dbgfmt/format_options.hpp"


namespace cxxdbg::dbg {


/// Represents node in watch tree info. Used to send data from implementation to client.
class watch_list_impl_tree_info_node {
    /// Type of vector of nodes
    typedef std::vector<watch_list_impl_tree_info_node> node_vector;

public:
    /// Type of node id
    typedef unsigned long node_id;

    /// Undefined node id value
    static constexpr node_id undef_node_id = ULONG_MAX;

    /// Default constructor
    watch_list_impl_tree_info_node():
        id_{0}, nchilds_{0} {}

    /// Constructor, makes node with specified id, name, value, type,
    /// and number of childs
    watch_list_impl_tree_info_node(node_id i,
                                   const std::wstring & nm,
                                   const std::wstring & v,
                                   const std::wstring & t,
                                   std::size_t nch,
                                   const source_position_info & p = {},
                                   const source_position_info & val_p = {},
                                   const source_position_info & type_p = {},
                                   unsigned long fmt_time = 0):
        id_(i), name_(nm), value_(v), type_(t),
        pos_(p), val_pos_(val_p), type_pos_(type_p), nchilds_(nch),
        fmt_time_{fmt_time} {}

    /// Returns node id
    node_id id() const { return id_; }

    /// Returns node name
    auto & name() const { return name_; }

    /// Returns node value
    auto & value() const { return value_; }

    /// Returns node type
    auto & type() const { return type_; }

    /// Returns node definition position
    auto & pos() const { return pos_; }

    /// Returns definition position of object pointed by value
    auto & val_pos() const { return val_pos_; }

    /// Retursn value type definition position
    auto & type_pos() const { return type_pos_; }

    /// Returns number of node childs
    std::size_t nchilds() const { return nchilds_; }

    /// Returns const reference to vector of child nodes
    const node_vector & childs() const { return childs_; }

    /// Adds child node
    void add_child(const watch_list_impl_tree_info_node & n) { childs_.push_back(n); }

    /// Returns format time
    unsigned long format_time() const { return fmt_time_; }

private:
    node_id id_;                    ///< Node id
    std::wstring name_;             ///< Node name
    std::wstring value_;            ///< Node value
    std::wstring type_;             ///< Node type
    source_position_info pos_;      ///< Node definition position
    source_position_info val_pos_;  ///< Definition position of objects pointed by value
    source_position_info type_pos_; ///< Type definition position
    std::size_t nchilds_;           ///< Number of childs
    node_vector childs_;            ///< Vector of child nodes
    unsigned long fmt_time_;        ///< Format time
};


/// Watch tree info. Used to send data from implementation to client.
class watch_list_impl_tree_info {
public:
    /// Tree node
    using node = watch_list_impl_tree_info_node;

private:
    /// Type of vector of nodes
    typedef std::vector<node> node_vector;

public:
    /// Constructs empty tree info
    watch_list_impl_tree_info() = default;

    /// Returns const reference to vector of root nodes
    const node_vector & root_nodes() const { return root_nodes_; }

    /// Adds root node
    void add_node(const node & n) { root_nodes_.push_back(n); }

private:
    node_vector root_nodes_;        ///< Vector of root nodes
};



/// \class watch_list_impl
/// Abstract implementation of a watch list
class watch_list_impl {
public:
    /// Watch tree info
    using tree_info = watch_list_impl_tree_info;

    /// Type of node id
    using node_id = watch_list_impl_tree_info_node::node_id;

    /// Undefined node id value
    static constexpr auto undef_node_id = watch_list_impl_tree_info_node::undef_node_id;

    /// Type of tree updated signal function
    typedef void tree_updated_func(const tree_info &);

    /// Type of node updated signal function
    typedef void node_updated_func(const typename tree_info::node &);

    /// Type of tree updated signal handler
    typedef std::function<tree_updated_func> tree_updated_handler;

    /// Type of node updated signal handler
    typedef std::function<void (const typename tree_info::node &)> node_handler;

    /// Destructor, destroys object
    virtual ~watch_list_impl() = default;

    /// Connects to tree updated signal. The signal is emitted after
    /// watch tree has been updated
    virtual signal_connection connect_tree_updated(const tree_updated_handler & handl) = 0;

    /// Exapnds tree node with specified id. Calls completion handler with expanded node
    /// after expanding is done.
    virtual void expand_node(node_id id, const node_handler & handl) = 0;

    /// Collapses tree node with specified id. Calls completion handler after completion.
    virtual void collapse_node(node_id id, const node_handler & handl) = 0;

    /// Sets format options. Calls completion handler after formatting tree with new options.
    virtual void set_fmt_opts(const dbgfmt::format_options & fmt_opts, const tree_updated_handler & handl) = 0;
};


}


