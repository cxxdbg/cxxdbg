// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "watch_list_server.hpp"
#include "cxxdbg/dbg/watch_list_impl.hpp"
#include "dbgfmt/fmt_result.hpp"
#include "cxxdbg/log/log.hpp"

#if false
#include <iostream>
#endif

namespace cxxdbg::dbg {


static const size_t max_count_default_value = 20;

namespace {


/// Converts fmt source position to source position info
source_position_info fmt_src_pos_to_pos(const dbgfmt::backend::source_position & pos) {
    if (static_cast<bool>(pos))
        return {pos.file(), pos.line()};

    return {};
}

}


watch_list_server::watch_list_server():
last_id_{0},
max_count_{max_count_default_value} {
}


// This function executes in core thread
watch_list_server::tree_info watch_list_server::make_values_tree() {
    CXXDBG_LOG_SCAT_TRACE(appcore, watch) << "watch_list_server::make_values_tree begin";

    // getting vector of variables in watch list
    auto vars = get_variables();

    CXXDBG_LOG_SCAT_TRACE(appcore, watch) << "watch_list_server::make_values_tree variables:";
    for (auto && var : vars) {
        CXXDBG_LOG_SCAT_TRACE(appcore, watch)
            << "watch_id = " << var.first << ", "
            << "name = '" << var.second.name() << "', "
            << "type = '" << var.second.type() << "'";
    }
    CXXDBG_LOG_SCAT_TRACE(appcore, watch) << "watch_list_server::make_values_tree variables end";

    // removing root nodes which are not in vars list
    remove_old_roots(vars);

    // acquiring existing root nodes
    for (auto var : vars) {
        auto wid = var.first;

        // trying find variable in current watch list
        auto wit = watches_.find(wid);
        if (wit == watches_.end()) {
            // variable is not in current watch list
            do_add_watch(wid, var.second);
        } else {
            // updating formatting results for watch node and its childs
            update_node_fmt_results(wit->second->node().get(), var.second);
        }
    }

    // building values tree
    tree_info tree;
    for (auto root_node : watches_) {
        tree.add_node(make_tree_info_node(root_node.second->node().get()));
    }

    return tree;
}


watch_list_server::tree_info::node
watch_list_server::expand_node_async(watch_list_server::node_id id) {
    auto it = nodes_.find(id);
    assert(it != nodes_.end() && "Can't find node");
    auto node = it->second;

    assert(node->fmt_res() && "formatting result should not be empty for node");

    // node may be already expanded
    if (node->childs().empty()) {
        // node is collapsed, creating childs nodes

        auto nchilds = node->fmt_res().childs_size();
        node->childs().reserve(nchilds);
        for (std::size_t i = 0; i < nchilds; ++i) {
            node->childs().push_back(make_new_tree_node(node->fmt_res().child_at(i)));
        }
    } else {
        // Node already expanded. Nothing to do. Number of child node must be equal to
        // number of childs in formatting result
        assert(node->fmt_res().childs_size() == node->childs().size() &&
               "node childs count inconsistency");
    }

    return make_node_info(node);
};

watch_list_server::tree_info::node
watch_list_server::collapse_node_async(watch_list_server::node_id id) {
    auto it = nodes_.find(id);
    assert(it != nodes_.end() && "Can't find node");
    auto * node = it->second;

    assert(node != nullptr && "Node is null");

    node->childs().clear();

    return make_node_info(node);
}


void watch_list_server::watch_queue_set_max_count(size_t count) {
    max_count_ = count;

    watch_queue_process_max_count();
}


typename watch_list_server::watch_id
watch_list_server::root_node_watch_id(node_id id) {
    auto it = root_nodes_.find(id);
    assert(it != root_nodes_.end() && "Can't find watch with specified root id");
    return it->second;
}


auto watch_list_server::add_watch(watch_id id, const dbgfmt::named_fmt_result & val) -> tree_info::node {
    // adding watch into list
    do_add_watch(id, val);

    // looking for added watch
    auto it = watches_.find(id);
    assert(it != watches_.end() && "can't find added watch");

    // formatting node
    return make_tree_info_node(it->second->node().get());
}


watch_list_server::tree_info::node
watch_list_server::make_watch_info(watch_id id, const dbgfmt::named_fmt_result & val) {
    // looking for watch with specified ID
    auto it = watches_.find(id);
    assert(it != watches_.end() && "can't find watch with specified id");

    // making node info
    return make_node_info(it->second->node().get());
}


void watch_list_server::do_add_watch(watch_id id, const dbgfmt::named_fmt_result & val) {
    assert(watches_.find(id) == watches_.end() && "watch is already in list");

    // first trying find existing watch in old watch cache by watch id
    auto keeper = watch_queue_find(id);

    if (!keeper) {
        // if there is no variable with watch id in cache when trying to find by (name, type) pair
        tree_node_key key = {val.name(), val.type()};
        keeper = watch_queue_find(key);
    }

    if (!keeper) {
        // variable not found in cache, creating new root node nad keeper for it
        tree_node_key key = {val.name(), val.type()};
        watch_list_server_tree_node_sp node = make_new_tree_node(val);
        keeper = std::make_shared<watch_keeper>(node, id, key);
        watch_queue_append(keeper);
    } else {
        // updating formatting result in existing tree
        update_node_fmt_results(keeper->node().get(), val);
    }

    watch_queue_acquire(keeper.get(), id);
}


void watch_list_server::remove_old_roots(const variables_map & vars) {
    // building lis of watches to delete from current watch map
    std::list<watch_id> del_watches;
    for (auto && [wid, keeper] : watches_) {
        if (vars.count(wid) == 0) {
            del_watches.push_back(wid);
        }
    }

    for (auto && wid : del_watches) {
        auto it = watches_.find(wid);
        assert(it != watches_.end() && "can't find watch");
        auto keeper = it->second;
        watches_.erase(it);

        remove_node_fmt_results(keeper->node().get());
        watch_queue_append(keeper);
        watch_queue_process_max_count();
    }
}


watch_list_server::watch_keeper_sp
watch_list_server::watch_queue_find(const tree_node_key & key) const {
    if (key.is_empty())
        return {};

    for (auto ptr : queue_) {
        if (ptr->key() == key) {
            return ptr;
        }
    }

    return {};
}


watch_list_server::watch_keeper_sp
watch_list_server::watch_queue_find(watch_id wid) const {
    auto it = watch_hash_.find(wid);
    if (it != watch_hash_.end())
        return *it->second;

    return {};
}


bool watch_list_server::watch_queue_append(watch_keeper_sp keeper) {
    if (watch_hash_.count(keeper->id()) > 0)
        return false;

    queue_.push_back(keeper);
    auto it = --(queue_.end());
    watch_hash_.insert({keeper->id(), it});

    return true;
}


void watch_list_server::watch_queue_acquire(watch_keeper * w, watch_id new_wid) {
    assert(w != nullptr && "pointer is nullptr");

    auto wid = w->id();

    watch_keeper_sp keeper = watch_queue_find(wid);
    if (!keeper && !w->key().is_empty()) {
        keeper = watch_queue_find(w->key());
    }

    if (!keeper) {
        return;
    }

    watch_queue_remove_from_queue(wid);
    keeper->set_wid(new_wid);

    auto res = watches_.insert({new_wid, keeper});
    assert(res.second && "watch already exists");
    root_nodes_.insert({keeper->node()->id(), new_wid});
}


void watch_list_server::watch_queue_remove_from_queue(unsigned long wid) {
    auto keeper = watch_queue_find(wid);
    if (!keeper) {
        return;
    }

    auto it = watch_hash_[wid];

    watch_hash_.erase(wid);
    queue_.erase(it);
}


void watch_list_server::watch_queue_process_max_count() {
    if (queue_.size() > max_count_) {
        auto diff = queue_.size() - max_count_;
        for (int i = 0; i < diff; ++i) {
            auto & item = queue_.front();

            watch_hash_.erase(item->id());
            queue_.pop_front();
        }
    }
}


watch_list_server_tree_node_sp
watch_list_server::make_new_tree_node(const dbgfmt::named_fmt_result & fres) {
    auto nd = std::make_shared<tree_node>(++last_id_, fres);
    nodes_.insert(std::make_pair(nd->id(), nd.get()));
    return nd;
}


// This function executes in core thread
watch_list_impl::tree_info::node watch_list_server::make_tree_info_node(tree_node * node) {
    const auto & fmt_res = node->fmt_res();
    assert(fmt_res && "formatting result for node must not be empty");

    // creating node
    tree_info::node res(node->id(),
                        fmt_res.name(),
                        fmt_res.val(),
                        fmt_res.type(),
                        fmt_res.childs_size(),
                        fmt_src_pos_to_pos(fmt_res.pos()),
                        fmt_src_pos_to_pos(fmt_res.val_pos()),
                        fmt_src_pos_to_pos(fmt_res.type_pos()),
                        fmt_res.format_time());

    // build child nodes
    for (auto && child : node->childs()) {
        res.add_child(make_tree_info_node(child.get()));
    }

    return res;
}


watch_list_impl::tree_info::node
watch_list_server::make_node_info(tree_node * target_node) {
    return make_tree_info_node(target_node);
}


void watch_list_server::update_node_fmt_results(tree_node * node, const dbgfmt::named_fmt_result & res) {
    node->set_fmt_res(res);

    auto res_childs_size = res.childs_size();

    if (!node->is_expanded()) {
        return;
    }

    // creating missing or removing redundant child nodes

    bool has_raw_data = res_childs_size > 0 && res.child_at(res_childs_size - 1).name() == L"[Raw Data]";

    if (node->childs().size() < res_childs_size) {
        // add children to node
        auto diff = res_childs_size - node->childs().size();

        std::vector<watch_list_server_tree_node_sp> new_children;
        new_children.reserve(diff);

        auto position_idx = node->childs().size();
        if (has_raw_data && position_idx > 0) {
            --position_idx;
        }

        for (size_t i = 0; i < diff; ++i) {
            new_children.push_back(make_new_tree_node({}));
        }

        node->childs().insert(node->childs().begin() + position_idx, new_children.begin(), new_children.end());
    } else if (res_childs_size < node->childs().size()) {
        // remove redundant children
        auto diff = node->childs().size() - res_childs_size;

        size_t from_position = has_raw_data ? res_childs_size - 1 : res_childs_size;

        // removing nodes from map of nodes
        for (size_t i = 0; i < diff; ++i) {
            tree_node_map::size_type count = nodes_.erase(node->childs().at(from_position + i)->id());
            assert(count && "Can't find node in node map");
        }

        // remove nodes from vector
        auto from_iterator = node->childs().begin() + from_position;
        node->childs().erase(from_iterator, from_iterator + diff);
    }

    // updating formatting results in child nodes
    assert(node->childs().size() == res_childs_size && "invalid number of child nodes");
    for (size_t i = 0; i < res_childs_size; ++i) {
        update_node_fmt_results(node->childs().at(i).get(), res.child_at(i));
    }
}


void watch_list_server::remove_node_fmt_results(tree_node * node) {
    node->set_fmt_res({});
    for (auto & ch : node->childs()) {
        remove_node_fmt_results(ch.get());
    }
}


}
