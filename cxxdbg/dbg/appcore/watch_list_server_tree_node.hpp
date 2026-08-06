// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <vector>

#include "cxxdbg/dbg/watch_list_impl.hpp"
#include "dbgfmt/fmt_result.hpp"


namespace cxxdbg::dbg {


/// \class tree_node represents node of internal tree
class watch_list_server_tree_node;

/// Type of shared pointer to tree node
typedef std::shared_ptr<watch_list_server_tree_node> watch_list_server_tree_node_sp;


/// Watch tree node
/// This class is used to keep internal tree nodes
class watch_list_server_tree_node {
public:
    /// Type node_id
    using node_id = dbg::watch_list_impl::node_id;

    /// Type of vector of tree nodes
    typedef std::vector<watch_list_server_tree_node_sp> tree_node_vector;

    /// Constructors new tree node with specified id, formatting result, and number of childs
    watch_list_server_tree_node(node_id i, const dbgfmt::named_fmt_result & fres):
        id_(i), fmt_res_{fres} {}

    /// Returns node id
    node_id id() const { return id_; }

    /// Returns reference to vector of childs
    tree_node_vector & childs() { return childs_; }

    /// Returns true if node is expanded else false
    bool is_expanded() const { return childs_.size() > 0; }

    /// Returns reference to formatting result
    auto & fmt_res() { return fmt_res_; }

    /// Returns const reference to formatting result
    auto & fmt_res() const { return fmt_res_; }

    /// Sets formatting result
    void set_fmt_res(const dbgfmt::named_fmt_result & fres) { fmt_res_ = fres; }

private:
    node_id id_;                    ///< Node id
    dbgfmt::named_fmt_result fmt_res_; ///< Fotmatting result for this node
    tree_node_vector childs_;       ///< Vector of child nodes
};


}

