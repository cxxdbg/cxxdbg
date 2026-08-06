// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file watch_list.cpp
/// Contains implementation of watch_list class.

#include "watch_list.hpp"
#include "cxxdbg/app/tree_view_model.hpp"
#include "watch_list_impl.hpp"
#include "cxxdbg/log/log.hpp"

#if false
#include <iostream>
#endif

#include <string>


namespace cxxdbg::dbg {


#if false
namespace debug_util {
using tree_info = watch_list_impl::tree_info;
using tree_node = tree_info::node;

void print_tree(const tree_info & tree_info) {
    std::function<void(const tree_node & node, int shift)> print_node =
            [&print_node](const tree_node & node, int shift) {
        std::cout << std::string(shift, ' ');
        std::wcout << L"(name=" << node.name() << L"; type=" << node.type() << L")" << std::endl;
        for (const tree_node & child : node.childs()) {
            print_node(child, shift + 2);
        }
    };

    std::cout << "========== tree info structure ==========" << std::endl;
    for (const tree_node & root : tree_info.root_nodes()) {
        print_node(root, 0);
    }
}
}
#endif



watch_list_tree_node::watch_list_tree_node(watch_list_tree_node * p,
                                           watch_list_impl::node_id i,
                                           const std::wstring & nm,
                                           const std::wstring & v,
                                           const std::wstring & t,
                                           std::size_t nchilds,
                                           unsigned long fmt_time):
        parent_{p}, id_(i), name_(nm), value_(v), type_(t),                 /*init properties*/
        changed_flag_(false),                                               /*init flags*/
        pos_{}, val_pos_{}, type_pos_{},                                    /*init positions*/
        fmt_time_{fmt_time} {
    reset_childs(nchilds);
}


const watch_list_tree_node * watch_list_tree_node::child_at(std::size_t index) const {
    assert(index < childs_size() && "Invalid child index");
    return childs_.at(index).get();
}


watch_list_tree_node * watch_list_tree_node::child_at(std::size_t index) {
    assert(index < childs_size() && "Invalid child index");
    return childs_.at(index).get();
}


void watch_list_tree_node::reset_childs(std::size_t n) {
    //childs_.clear();
    std::size_t size = childs_.size();

    if (size <= n) {
        childs_.reserve(n);

        for (std::size_t i = size; i < n; ++i) {
            watch_list_tree_node_sp child_node(new watch_list_tree_node{this});
            childs_.push_back(child_node);
        }
    } else {
        childs_.erase(childs_.begin() + n, childs_.end());
    }
}

void watch_list_tree_node::reset_childs_preserve_raw_data(std::size_t n) {
    std::size_t size = childs_.size();

    if (size <= n) {
        childs_.reserve(n);
        for (std::size_t i = size; i < n; ++i) {
            watch_list_tree_node_sp child_node(new watch_list_tree_node{this});
            childs_.insert(childs_.end() - 1, child_node);
        }
    } else {
        childs_.erase(childs_.end() - 2, childs_.end() - 1);
    }
}


void watch_list_tree_node::set_child_at(std::size_t i, watch_list_tree_node::ptr node) {
    assert(i < childs_.size() && "Index out of bounds.");
    childs_[i] = node;
}


watch_list_tree_node * watch_list_tree_node::root() {
    auto * node = this;
    while(node && node->parent())
        node = node->parent();

    return node;
}


const watch_list_tree_node * watch_list_tree_node::root() const {
    auto * node = this;
    while(node && node->parent())
        node = node->parent();

    return node;
}


watch_list::watch_list(watch_list_impl * i):
impl_(i) {
    connect_impl();
}


watch_list::~watch_list() {
}


void watch_list::set_impl(watch_list_impl * impl) {
    do_set_impl(impl, true);
}


std::size_t watch_list::root_nodes_size() const {
    return root_nodes_.size();
}


watch_list::const_root_node_iterator watch_list::root_nodes_begin() const {
    return const_root_node_iterator(root_nodes_.begin());
}


watch_list::const_root_node_iterator watch_list::root_nodes_end() const {
    return const_root_node_iterator(root_nodes_.end());
}


void watch_list::expand_node(const tree_node * cnode) {
    auto node = const_cast<tree_node*>(cnode);

    CXXDBG_LOG_SCAT_TRACE(dbg, watchlist) << "expand node: " << node;

    assert(node != nullptr && "Null node pointer");
    assert(node->id() != undef_node_id && "can't expand node with undefined ID");
    assert(node->childs_size() != 0 && "can't expand empty nodes");
    assert(node->expand_state() == tree_view_expand_state::collapsed && "node already expanded");

    // setting expand state visible to tree view
    node->set_expand_state(tree_view_expand_state::expanded);

    // adding node id to set of nodes being expanded or collapsed
    assert(node->id() != undef_node_id && "invalid node id");
    inc_node_expand_collapse_ops(node->id());

    // expanding node in implementation
    impl().expand_node(node->id(), [this, node](auto && node_info) {
        this->on_node_expand_complete(const_cast<tree_node*>(node), node_info);
    });
}


void watch_list::collapse_node(const tree_node * cnode) {
    auto node = const_cast<tree_node*>(cnode);

    CXXDBG_LOG_SCAT_TRACE(dbg, watchlist) << "collapse node: " << node;

    assert(node != nullptr && "Null node pointer");
    assert(node->childs_size() != 0 && "can't collapse empty nodes");
    assert(node->expand_state() == tree_view_expand_state::expanded && "not not expanded");

    // setting expand state visible to tree view
    node->set_expand_state(tree_view_expand_state::collapsed);

    // adding node id to set of nodes being expanded or collapsed
    assert(node->id() != undef_node_id && "invalid node id");
    inc_node_expand_collapse_ops(node->id());

    // collapsing node in implementation
    impl().collapse_node(node->id(), [this, node](auto && node_info) {
        this->on_node_collapse_compete(const_cast<tree_node*>(node), node_info);
    });
}


void watch_list::set_fmt_opts(const dbgfmt::format_options & opts) {
    fmt_opts_ = opts;

    if (!has_impl()) {
        return;
    }

    impl().set_fmt_opts(opts, [this](auto && tree) {
        on_watch_tree_updated(tree, false);
    });
}


void watch_list::add_root_node(const tree_node_sp & node) {
    root_nodes_.push_back(node);

    if (node->id() != undef_node_id) {
        auto res = root_nodes_map_.insert(std::make_pair(node->id(), node.get()));
        assert(res.second);
    }

    emit_root_node_added(node.get());
}


watch_list::tree_node_sp watch_list::remove_root_node(const tree_node * node) {
    auto it = std::find_if(root_nodes_.begin(), root_nodes_.end(), [node](const auto & n) {
        return n.get() == node;
    });
    assert(it != root_nodes_.end() && "can't find root node");

    emit_root_node_removed(node);
    if (node->id() != undef_node_id)
        root_nodes_map_.erase(node->id());

    // removing node from list of nodes being expanded or collapsed (if it's there)
    exp_coll_nodes_.erase(node->id());

    auto res = *it;
    root_nodes_.erase(it);
    return res;
}


std::tuple<bool, bool>
watch_list::update_node_values(tree_node * node, const watch_list_impl_tree_info_node & node_info) {
    bool changed = false;
    bool pos_changed = false;

    // updating node values if needed

    if (node->name() != node_info.name()) {
        node->set_name(node_info.name());
        changed = true;
    }

    if (node->value() != node_info.value()) {
        node->set_value(node_info.value());
        changed = true;
    }

    if (node->type() != node_info.type()) {
        node->set_type(node_info.type());
        changed = true;
    }

    // updating node position values if needed

    if (node->pos() != node_info.pos()) {
        node->set_pos( node_info.pos() );
        pos_changed = true;
    }

    if (node->val_pos() != node_info.val_pos()) {
        node->set_val_pos( node_info.val_pos() );
        pos_changed = true;
    }

    if (node->type_pos() != node_info.type_pos()) {
        node->set_type_pos( node_info.type_pos() );
        pos_changed = true;
    }

    // updating format time if not set or node values changed
    if (node->format_time() == ULONG_MAX || changed || pos_changed) {
        node->set_format_time(node_info.format_time());
        changed = true;
    }

    return {changed, pos_changed};
}


void watch_list::update_node(tree_node * node,
                             const watch_list_impl::tree_info::node & node_info,
                             bool mark_changed_nodes) {

    // setting node ID for not root nodes
    if (node->parent() != nullptr) {
        node->set_id(node_info.id());
    } else {
        assert(node_info.id() != undef_node_id && "node ID should not be undefined");
    }

    // updating node values
    auto [changed, pos_changed] = update_node_values(node, node_info);

    // setting node expand state
    tree_view_expand_state exp_state = node->expand_state();
    if (node_info.childs().size() > 0) {
        exp_state = tree_view_expand_state::expanded;
    } else {
        // don't set collapsed state if number of child nodes is zero
        if (node_info.nchilds() != 0) {
            exp_state = tree_view_expand_state::collapsed;
        }
    }
    bool exp_state_changed = node->expand_state() != exp_state;
    node->set_expand_state(exp_state);

    // setting/clearing node marked flag
    bool marked_changed = false;
    if (mark_changed_nodes) {
        if (!changed && node->is_marked()) {
            // node chilled out
            node->set_marked(false);
            marked_changed = true;
        } else if (changed) {
            node->set_marked(true);
            marked_changed = true;
        }
    }

    // emitting changed signal for node
    if (changed || pos_changed || exp_state_changed || marked_changed) {
        emit_node_changed(node);
    }

    if (node->childs_size() != node_info.nchilds()) {
        // node childs were changed. reset_childs will remove old or add new nodes
        reset_childs(node, node_info);
    }

    if (node_info.childs().size() != 0) {
        // updating node childs
        assert(node_info.nchilds() == node_info.childs().size() && "Node childs inconsistency");

        for (std::size_t i = 0, e = node_info.childs().size(); i < e; ++i) {
            update_node(node->child_at(i), node_info.childs().at(i), mark_changed_nodes);
        }
    }
}


watch_list::node_id watch_list::get_node_id(const tree_node * node) {
    return node->id();
}


void watch_list::set_root_node_id(tree_node * node, node_id new_id) {
    assert(node->id() == undef_node_id && "ID of node is already set");
    node->set_id(new_id);
    auto res = root_nodes_map_.emplace(new_id, node);
    assert(res.second && "node with specified ID is already in root node map");
}


void watch_list::on_watch_tree_updated(const watch_list_impl::tree_info & tree, bool mark_changed_nodes) {
#if 0
    std::cout << __FUNCTION__ << std::endl;
    debug_util::print_tree(tree);
#endif

    // removing root nodes which are not in new tree
    for (auto it = std::begin(root_nodes_), end = std::end(root_nodes_); it != end;) {
        tree_node * node = it->get();

        // TODO: try remove n^2 complexity
        auto it2 = std::find_if(tree.root_nodes().begin(), tree.root_nodes().end(),
                                [this, node](const watch_list_impl::tree_info::node & n) {
            return node->id() == n.id();
        });

        auto old_it = it++;

        if (it2 == tree.root_nodes().end()) {
            remove_root_node(node);
        }
    }


    // adding new nodes and updating existing nodes
    for (auto & node_info : tree.root_nodes()) {
        assert(node_info.id() != watch_list_impl::undef_node_id && "Invalid node id");

        tree_node * node = nullptr;

        // looking for existing root node with same id
        auto node_it = root_nodes_map_.find(node_info.id());
        if (node_it != root_nodes_map_.end()) {
            node = node_it->second;
        }

        if (node == nullptr) {
            // new node
            process_new_node(node_info);
        } else {
            // updating existing node
            update_node(node, node_info, mark_changed_nodes);
        }
    }
}


void watch_list::reset_childs(tree_node * node, const watch_list_impl::tree_info::node & node_info) {

    std::size_t size = node->childs_size();
    std::size_t n = node_info.nchilds();

    bool keep_in_mind_raw_data = node->childs_size() > 0 && n > 0 &&
            node->child_at(size - 1)->name() == L"[Raw Data]";

    // impossible to find out whether [Raw Data] will remain in children after update
    // consider it will remain or will not appear

    if (!keep_in_mind_raw_data) {
        if (n < size) { // size > 0 => (size - 1) is correct
            emit_before_removed(node, n, size - 1);

            node->reset_childs(n);

            emit_after_removed(node, n, size - 1);

        } else {
            emit_before_added(node, size, n - 1);

            node->reset_childs(n);

            emit_after_added(node, size, n - 1);
        }
    } else {
        if (n < size) {
            emit_before_removed(node, n - 1, size - 2);

            node->reset_childs_preserve_raw_data(n);

            emit_after_removed(node, n - 1, size - 2);

        } else {
            emit_before_added(node, size - 1, n - 2);

            node->reset_childs_preserve_raw_data(n);

            emit_after_added(node, size - 1, n - 2);
        }
    }
}


void watch_list::process_new_node(const watch_list_impl::tree_info::node & node_info) {
    using tree_node_info = watch_list_impl::tree_info::node;
    std::function<tree_node_sp (watch_list_tree_node *, const tree_node_info &)> build_tree =
            [&build_tree](watch_list_tree_node * parent, const tree_node_info & info) -> tree_node_sp {
                tree_node_sp new_node(new tree_node(parent,
                                                    info.id(),
                                                    info.name(),
                                                    info.value(),
                                                    info.type(),
                                                    info.nchilds(),
                                                    info.format_time()));

                auto exp_state = info.childs().size() > 0 ?
                     tree_view_expand_state::expanded :
                     tree_view_expand_state::collapsed;
                new_node->set_expand_state(exp_state);
                new_node->set_pos(info.pos());
                new_node->set_val_pos(info.val_pos());
                new_node->set_type_pos(info.type_pos());

                for (size_t i = 0; i < info.childs().size(); ++i) {
                    auto & child_info = info.childs()[i];
                    auto new_child = build_tree(new_node.get(), child_info);
                    new_node->set_child_at(i, new_child);
                };
                return new_node;
            };

    tree_node_sp new_node = build_tree(nullptr, node_info);
    add_root_node(new_node);

    update_tree_expand_state(new_node.get());
}


void watch_list::update_tree_expand_state(watch_list::tree_node * node) {
    if (node->expand_state() == tree_view_expand_state::expanded) {
        // sending changed signal to expand node in views
        emit_node_changed(node);
    }

    // processing child nodes
    for (size_t i = 0, e = node->childs_size(); i < e; ++i) {
        update_tree_expand_state(node->child_at(i));
    }
}


void watch_list::clear_node(const tree_node * cnode, bool clear_id) {
    auto node = const_cast<tree_node*>(cnode);
    
    // clearing node ID
    if (clear_id && node->id() != undef_node_id) {
        auto cnt = root_nodes_map_.erase(node->id());
        assert(cnt != 0 && "node not found in root node map");
        node->set_id(undef_node_id);
    }


    // clearing node values

    node->set_value({});
    node->set_type({});
    node->set_pos({});
    node->set_val_pos({});
    node->set_type_pos({});
    node->set_format_time(ULONG_MAX);

    emit_node_changed(node);

    // removing node childs
    remove_childs(node);
}


void watch_list::do_set_impl(watch_list_impl * i, bool remove_roots) {
    disconnect_impl();

    if (remove_roots) {
        // removing all root nodes
        while (root_nodes_size() != 0) {
            remove_root_node(root_nodes_.back().get());
        }
    }

    impl_ = i;
    connect_impl();

    if (has_impl()) {
        impl().set_fmt_opts(fmt_opts_, [](auto && ...){});
    }
}


void watch_list::connect_impl() {
    if (!has_impl()) {
        return;
    }    

    watch_tree_updated_con_ = impl().connect_tree_updated([this](const watch_list_impl::tree_info & tree) {
        on_watch_tree_updated(tree, true);
    });
}


void watch_list::disconnect_impl() {
    if (!has_impl()) {
        return;
    }

    // disconnecting form implementation signals
    watch_tree_updated_con_.disconnect();
}


void watch_list::remove_childs_from_exp_col(tree_node * node) {
    for (int i = 0, e = node->childs_size(); i < e; ++i) {
        auto ch = node->child_at(i);

        if (ch->id() != undef_node_id) {
            exp_coll_nodes_.erase(ch->id());
        }

        remove_childs_from_exp_col(ch);
    }
}


void watch_list::remove_childs(tree_node * node) {
    auto nchilds = node->childs_size();

    if (nchilds == 0) {
        return;
    }

    remove_childs_from_exp_col(node);
    emit_before_removed(node, 0, nchilds - 1);
    node->reset_childs(0);
    emit_after_removed(node, 0, nchilds - 1);
}


void watch_list::on_node_expand_complete(tree_node * node, const watch_list_impl::tree_info::node & node_info) {
    // Node pointer may be not correct here if node was removed after epxanding was started.
    // We need check node return value of the dec_node_expand_collapse_ops before access to node.

    // decreasing expand/collapse operation counter for node
    if (!dec_node_expand_collapse_ops(node_info.id())) {
        // node was removed
        return;
    }

    // checking that node values were not changed after expand
    auto [changed, pos_changed] = update_node_values(node, node_info);
    assert(!changed && !pos_changed && "node values should not change after expand");

    // node must be expanded in implementation
    assert(node_info.childs().size() == node_info.nchilds() && "node must be expanded in impl");

    // number of child nodes can't change after expand
    assert(node->childs_size() == node_info.nchilds());

    // number of childs of expanded node must be > 0
    assert(node->childs_size() > 0 && "can't expand empty node");

    // updating child nodes
    for (size_t i = 0, e = node_info.childs().size(); i < e; ++i) {
        auto child = node->child_at(i);
        const auto & child_info = node_info.childs().at(i);

        // updating child ID
        assert(child->id() == undef_node_id && "ID of child node must be undefined");
        child->set_id(child_info.id());

        // updating node values
        update_node_values(child, child_info);

        // sending changed signal for child
        emit_node_changed(child);

        // adding new empty childs for child node
        assert(child->childs_size() == 0 && "child of expanded node must not have childs");
        assert(child_info.childs().size() == 0 && "child of expanded node must not be expanded");
        if (child_info.nchilds() != 0) {
            emit_before_added(child, 0, child_info.nchilds() - 1);
            child->reset_childs(child_info.nchilds());
            emit_after_added(child, 0, child_info.nchilds() - 1);
        }
    }
}


void watch_list::on_node_collapse_compete(tree_node * node, const watch_list_impl_tree_info_node & node_info) {
    // Node pointer may be not correct here if node was removed after collapsing was started.
    // We need check node return value of the dec_node_expand_collapse_ops before access to node.

    // decreasing expand/collapse operation counter for node
    if (!dec_node_expand_collapse_ops(node_info.id())) {
        // node was removed
        CXXDBG_LOG_SCAT_TRACE(dbg, watchlist) << "on node collapse complete: " << node << " <removed>";
        return;
    }

    CXXDBG_LOG_SCAT_TRACE(dbg, watchlist) << "on node collapse complete: " << node << " '" << node->name() << "'";

    // checking that node values were not changed after collapse
    auto [changed, pos_changed] = update_node_values(node, node_info);
    assert(!changed && !pos_changed && "node values should not change after collapse");

    // node must be collapsed in implementation
    assert(node_info.childs().size() == 0 && "node must be collapse in impl");

    // number of child nodes can't change after collapse
    assert(node->childs_size() == node_info.nchilds());

    // number of childs of collapsed node must be > 0
    assert(node->childs_size() > 0 && "can't collapse empty node");

    // updating child nodes
    for (size_t i = 0, e = node->childs_size(); i < e; ++i) {
        auto child = node->child_at(i);

        // updating child ID
        child->set_id(undef_node_id);

        // updating node values
        child->set_name({});
        child->set_value({});
        child->set_type({});
        child->set_pos({});
        child->set_val_pos({});
        child->set_type_pos({});

        // we have to explicitly collapse child nodes because tree view preserve
        // expand state of child nodes of collapsed node (CXXDBG-695)
        child->set_expand_state(tree_view_expand_state::collapsed);

        // sending changed signal for child
        emit_node_changed(child);

        // removing child nodes of child node if any
        auto nchilds = child->childs_size();
        if (nchilds > 0) {
            emit_before_removed(child, 0, nchilds - 1);
            child->reset_childs(0);
            emit_after_removed(child, 0, nchilds - 1);
        }
    }
}


void watch_list::emit_root_node_added(const tree_node * node) {
    CXXDBG_LOG_SCAT_TRACE(dbg, watchlist) << "root node added: " << node << " '" << node->name() << "'";
    root_node_added_(node);
}


void watch_list::emit_root_node_removed(const tree_node * node) {
    CXXDBG_LOG_SCAT_TRACE(dbg, watchlist) << "root node removed: " << node << " '" << node->name() << "'";
    root_node_removed_(node);
}


void watch_list::emit_node_changed(const tree_node * node) {
    CXXDBG_LOG_SCAT_TRACE(dbg, watchlist) << "node changed: " << node << " '" << node->name() << "'";
    node_changed_(node);
}


void watch_list::emit_before_added(tree_node * node, size_t first, size_t last) {
    CXXDBG_LOG_SCAT_TRACE(dbg, watchlist) << "before added: " << node << " '" << node->name() << "' "
                                    << "first = " << first << ", last = " << last;
    before_added_(node, first, last);
}


void watch_list::emit_after_added(tree_node * node, size_t first, size_t last) {
    CXXDBG_LOG_SCAT_TRACE(dbg, watchlist) << "after added: " << node << " '" << node->name() << "' "
                                    << "first = " << first << ", last = " << last;
    after_added_(node, first, last);
}


void watch_list::emit_before_removed(tree_node * node, size_t first, size_t last) {
    CXXDBG_LOG_SCAT_TRACE(dbg, watchlist) << "before removed: " << node << " '" << node->name() << "' "
                                    << "first = " << first << ", last = " << last;
    before_removed_(node, first, last);
}


void watch_list::emit_after_removed(tree_node * node, size_t first, size_t last) {
    CXXDBG_LOG_SCAT_TRACE(dbg, watchlist) << "after removed: " << node << " '" << node->name() << "' "
                                    << "first = " << first << ", last = " << last;
    after_removed_(node, first, last);
}


void watch_list::inc_node_expand_collapse_ops(node_id id) {
    assert(id != undef_node_id && "node id must not be undefined here");
    ++exp_coll_nodes_[id];
}


bool watch_list::dec_node_expand_collapse_ops(node_id id) {
    assert(id != undef_node_id && "node id must not be undefined here");
    auto it = exp_coll_nodes_.find(id);
    if (it == exp_coll_nodes_.end()) {
        return false;
    }

    assert(it->second > 0 && "node operation counter must be greater than 0");
    --it->second;
    if (it->second == 0) {
        exp_coll_nodes_.erase(it);
    }

    return true;
}


}
