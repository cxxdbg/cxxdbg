// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file custom_watch_list.hpp
/// Contains definition of the custom_watch_list class.

#pragma once

#include "cxxdbg/dbg/watch_list_impl.hpp"
#include "watch_list.hpp"
#include <map>
#include <set>


namespace cxxdbg::dbg {


class custom_watch_list_impl;


/// Watch list which displays list of values specified by user
class custom_watch_list: public watch_list {
public:
    /// Constructors watch list with specified pointer
    /// to custom watch list implementation
    custom_watch_list(custom_watch_list_impl * impl);

    /// Destructor, destroys object
    ~custom_watch_list();

    /// Sets pointer to implementation
    void set_impl(watch_list_impl * impl) override;

    /// Adds expression to watch list
    void add_watch(const std::wstring & expr);

    /// Sets watch expression for watch with specified root node
    void set_watch(const tree_node * n, const std::wstring & expr);

    /// Removes expression with specified root node id from watch list
    void remove_watch(const tree_node * n);

private:
    /// Returns reference to implementation
    auto & impl() {
        assert(impl_ != nullptr && "implementation is null");
        return *impl_;
    }

    /// Returns true if pointer to implementation is not null
    bool has_impl() const { return impl_ != nullptr; }

    /// Called when adding new watch is complete in implementation
    void on_watch_added(tree_node * node,
                        const watch_list_impl_tree_info_node & node_info,
                        bool mark_changed);

    custom_watch_list_impl * impl_;     ///< Pointer to implementation
    
    /// List of removed nodes which are waiting for removal in implementation
    std::list<tree_node_sp> removed_nodes_;

    /// Map of renamed nodes
    std::map<const tree_node*, std::wstring> renamed_nodes_;
};


}


