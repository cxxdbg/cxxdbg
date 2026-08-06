// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "watch_list_server_tree_node.hpp"
#include "tree_node_key.hpp"
#include "cxxdbg/dbg/watch_list_impl.hpp"
#include "dbgfmt/context.hpp"
#include "dbgfmt/fmt_result.hpp"
#include <unordered_map>


namespace cxxdbg::dbg {


class watch_list_server : public virtual watch_list_impl {
public:
    /// Alias for tree node
    using tree_node = watch_list_server_tree_node;

    /// Shared pointer to tree node
    using tree_node_sp = std::shared_ptr<tree_node>;

private:
    /// \class watch_keeper keeps single watch tree node
    class watch_keeper {
        friend class watch_queue;
    public:
        typedef unsigned long watch_id;

        using key_type = tree_node_key;

        /// Constructor makes new watch keeper
        /// should be used from watch_queue, cannot be created independantly
        watch_keeper(tree_node_sp node, watch_id wid, const key_type & key):
            node_(node), wid_(wid), key_(key) {}

        /// Returns watch id
        watch_id id() const { return wid_; }

        /// Sets watch id
        void set_wid(watch_id wid) { wid_ = wid; }

        /// Returns const reference to watch key
        const key_type & key() const { return key_; }

        /// Sets key
        void set_key(const key_type & key) { key_ = key; }

        /// Returns ptr to node
        watch_list_server_tree_node_sp node() { return node_; }

    private:
        watch_list_server_tree_node_sp node_; ///< Root node pointer
        watch_id wid_;      ///< Watch id
        key_type key_;      ///< Watch key: {name, type} pair
    };


    typedef std::shared_ptr<watch_keeper> watch_keeper_sp;


public:
    /// Type of watch id
    typedef unsigned long watch_id;

    /// Constructor
    watch_list_server();

    /// Destructor
    ~watch_list_server() override = default;

    /// Updates watch tree and makes values tree. Should be called in core thread.
    tree_info make_values_tree();

    /// Expands node and returns node subtree
    tree_info::node expand_node_async(node_id id);

    /// Collapses node and returns node subtree
    tree_info::node collapse_node_async(node_id id);

    /// Returns queue items count
    size_t watch_queue_items_count() const { return queue_.size(); }

    /// returns max items count for the list
    size_t watch_queue_max_count() const { return max_count_; }

    /// sets max items count for the list
    void watch_queue_set_max_count(size_t count);

protected:
    /// Returns watch id for root node with specified id
    watch_id root_node_watch_id(node_id id);

    /// Type of map of variables
    using variables_map = std::map<watch_id, dbgfmt::named_fmt_result>;

    /// Adds variable into current watch list. Returns tree for new watch node
    tree_info::node add_watch(watch_id id, const dbgfmt::named_fmt_result & val);

    /// Makes node info for watch with specified ID
    tree_info::node make_watch_info(watch_id id, const dbgfmt::named_fmt_result & val);

private:
    /// Type of map from node id to node
    typedef std::map<node_id, tree_node*> tree_node_map;

    /// Returns map of (id, value) in watch list
    virtual variables_map get_variables() = 0;

    /// Adds variable into current watch list
    void do_add_watch(watch_id id, const dbgfmt::named_fmt_result & val);

    /// Removes nodes that are not in new list of variables and moves them
    /// to queue of old watches
    void remove_old_roots(const variables_map & vars);

    /// Creates watch keeper item
    watch_keeper_sp watch_queue_create_item(watch_list_server_tree_node_sp node, watch_id wid, const tree_node_key & key = {});

    /// Finds keeper in queue by key
    watch_keeper_sp watch_queue_find(const tree_node_key & key) const;

    /// Finds keeper in queue by watch id
    watch_keeper_sp watch_queue_find(watch_id wid) const;

    /// Appends watch keeper to the queue
    bool watch_queue_append(watch_keeper_sp keeper);

    /// Moves keeper from queue to watch map
    void watch_queue_acquire(watch_keeper * w, watch_id new_wid);

    /// Removes keeper from queue
    void watch_queue_remove_from_queue(watch_id wid);

    /// Removes redundant items
    void watch_queue_process_max_count();

    /// Makes new tree node with specified formatting result and number of childs
    watch_list_server_tree_node_sp make_new_tree_node(const dbgfmt::named_fmt_result & fres);

    /// Builds values tree for specified tree node
    tree_info::node make_tree_info_node(tree_node * node);

    /// Builds values subtree for specified tree node
    tree_info::node make_node_info(tree_node * node);

    /// Recursively updates formatting results in node and its childs
    /// Removes redundant childs or adds new childs
    void update_node_fmt_results(tree_node * node, const dbgfmt::named_fmt_result & res);

    /// Recursively removes formatting results in node and its childs
    void remove_node_fmt_results(tree_node * node);


    node_id last_id_;                               ///< Last node id
    tree_node_map nodes_;                           ///< Map from node ids to nodes

    std::map<watch_id, watch_keeper_sp> watches_;   ///< Map of current watch items
    std::map<node_id, watch_id> root_nodes_;        ///< Map from root node id to its watch id

    typedef std::list<watch_keeper_sp> queue_type;
    typedef queue_type::iterator queue_iterator;

    size_t max_count_;  ///< Max item count in queue
    queue_type queue_;  ///< Queue implementation (std::list)

    /// Map: watch id -> queue iterator
    std::unordered_map<watch_id, queue_iterator> watch_hash_;
};


}


