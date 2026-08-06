// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file custom_watch_list.cpp
/// Contains implementation of the custom_watch_list class.

#include "custom_watch_list.hpp"
#include "custom_watch_list_impl.hpp"

#include <iostream>


namespace cxxdbg::dbg {


custom_watch_list::custom_watch_list(custom_watch_list_impl * impl):
watch_list{impl},
impl_{impl} {
}


custom_watch_list::~custom_watch_list() {
}


void custom_watch_list::set_impl(watch_list_impl * new_impl) {
    if (new_impl == nullptr) {
        impl_ = nullptr;
    } else {
        auto c_new_impl = dynamic_cast<custom_watch_list_impl*>(new_impl);
        assert(c_new_impl && "implementation must be a custom watch implementation");
        impl_ = c_new_impl;
    }

    // setting pointer to implementation in base class
    watch_list::do_set_impl(impl_, false);

    if (!has_impl()) {
        // new implementation is null, we need clear values of all root nodes
        for(auto it = root_nodes_begin(), end = root_nodes_end(); it != end; ++it) {
            clear_node(*it, true);
        }

        return;
    }

    // adding all watches in new implementation

    if (root_nodes_size() == 0) {
        return;
    }

    for(auto it = root_nodes_begin(), end = root_nodes_end(); it != end; ++it) {
        auto node = *it;
        impl().add_watch(node->name(), [this, node](auto && node_info) {
            on_watch_added(const_cast<tree_node*>(node), node_info, true);
        });
    }
}


void custom_watch_list::add_watch(const std::wstring & expr) {
    // adding root node in watch_list
    auto node_sp = std::make_shared<tree_node>(nullptr, undef_node_id, expr);
    auto node = node_sp.get();
    add_root_node(node_sp);

    if (has_impl()) {
        // adding watch in implementation
        impl().add_watch(expr, [this, node](auto && node_info) {
            on_watch_added(const_cast<tree_node*>(node), node_info, false);
        });
    }
}


void custom_watch_list::set_watch(const tree_node * n, const std::wstring & expr) {
    assert(n != nullptr && "invalid watch node");
    assert(!expr.empty() && "watch expression should not be empty");

    // doing nothing if node expression is same
    if (n->name() == expr) {
        return;
    }

    tree_node * node = const_cast<tree_node*>(n);
    auto id = node->id();
    auto orig_name = node->name();

    // setting new node name and clearing node values and childs
    node->set_name(expr);
    clear_node(node, false);

    // sending update request to implementation
    if (id != undef_node_id) {
        impl().set_watch(id, expr, [this, node](auto && node_info) {
            update_node(const_cast<tree_node*>(node), node_info, false);
        });
    } else {
        // We don't know ID of node yet. Adding node to map of renamed nodes.
        // We will process it in on_watch_added handler. If node is already
        // in map of renamed nodes then we should use old original name
        renamed_nodes_.emplace(node, orig_name);
    }
}


void custom_watch_list::remove_watch(const tree_node * n) {
    // saving node id
    auto id = get_node_id(n);

    // removing root from watch_list
    auto node_sp = remove_root_node(n);

    if (id != undef_node_id) {
        // removing watch from implementation
        impl().remove_watch(id);
    } else {
        // we can't delete watch from implementation because we don't know
        // id of root node. Put shared pointer to removed node into set of removed
        // nodes. We will process them in on_watch_added completion handler
        removed_nodes_.emplace_back(node_sp);

        // removing node from set of renamed nodes if it's there
        renamed_nodes_.erase(node_sp.get());
    }
}


void custom_watch_list::on_watch_added(tree_node * node, const watch_list_impl_tree_info_node & node_info, bool mark_changed) {
    // checking for removed node
    auto it = std::find_if(removed_nodes_.begin(), removed_nodes_.end(), [node](auto && n) { return n.get() == node; });
    if (it != removed_nodes_.end()) {
        // node was removed with remove_watch before we received node ID from implementation.
        // removing node from implementation here
        impl().remove_watch(node_info.id());
        removed_nodes_.erase(it);
        return;
    }

    // setting node ID
    set_root_node_id(node, node_info.id());

    // checking for renamed node
    auto it2 = renamed_nodes_.find(node);
    if (it2 != renamed_nodes_.end()) {
        // node was renamed before we received node ID from implementation, renaming it here

        impl().set_watch(node_info.id(), node->name(), [this, node, mark_changed](auto && node_info) {
            update_node(node, node_info, mark_changed);
        });

        renamed_nodes_.erase(it2);
        return;
    }

    // updating node
    update_node(const_cast<tree_node*>(node), node_info, mark_changed);
}


}
