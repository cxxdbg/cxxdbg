// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file watch_model.cpp
/// Contains implementation of the watch_model class.

#include "watch_model.hpp"
#include "watch_list.hpp"
#include "dbgfmt/format_options.hpp"
#include "cxxdbg/log/log.hpp"


namespace cxxdbg::dbg {


watch_model::watch_model(watch_list & wlist):
watch_{wlist} {
    // connecting to watch list signal

    // add for root nodes
    root_node_added_con_ = wlist.connect_root_node_added([this](auto node) {
        this->on_root_node_added(node);
    });

    // remove for root node
    root_node_removed_con_ = wlist.connect_root_node_removed([this](auto node) {
        this->on_root_node_removed(node);
    });

    node_changed_con_ = wlist.connect_node_changed([this](auto node) {
        this->on_node_changed(node);
    });


    // add/remove children signals

    before_added_con_ = wlist.connect_before_added([this](auto * node, std::size_t first, std::size_t last) {
        this->before_added()(make_index(node), first, last);
    });

    after_added_con_ = wlist.connect_after_added([this](auto * node, std::size_t first, std::size_t last) {
        this->after_added()(make_index(node), first, last);
    });

    before_removed_con_ = wlist.connect_before_removed([this](auto * node, std::size_t first, std::size_t last) {
       this->before_removed()(make_index(node), first, last);
    });

    after_removed_con_ = wlist.connect_after_removed([this](auto * node, std::size_t first, std::size_t last) {
       this->after_removed()(make_index(node), first, last);
    });
}


watch_model::~watch_model() {
}


std::size_t watch_model::columns_size() const {
    return 4;
}


std::wstring watch_model::column_name(std::size_t index) const {
    switch (index) {
    case 0:
        return L"Name";
    case 1:
        return L"Value";
    case 2:
        return L"Type";
    case 3:
        return L"Format time";
    default:
        assert(false && "invalid column index");
        return std::wstring();
    }
}


std::size_t watch_model::childs_size(const row_index & row) const {
    if (!row) {
        // count of root nodes
        return root_nodes_.size();
    }

    const watch_list_tree_node * node = row_node(row);
    assert(node != nullptr && "invalid row node");

    if (node == update_childs_node_)
        return update_childs_node_size_;

    return node->childs_size();
}


watch_model::row_index
watch_model::child(const row_index & row, std::size_t index) const {
    if (!row) {
        // root node
        assert(index < root_nodes_.size() && "invalid root node index");
        return make_node_index(root_nodes_[index]);
    }

    const watch_list_tree_node * node = row_node(row);
    assert(index < node->childs_size() && "invalid child index");
    return make_node_index(node->child_at(index));
}


std::size_t watch_model::index(const row_index & row) const {
    assert(row && "invalid row");
    return index_in_parent(row_node(row));
}


watch_model::row_index watch_model::parent(const row_index & r) const {
    return make_node_index(row_node(r)->parent());
}


static std::wstring str_to_wstr(const std::string & str) {
    std::wstring res;
    res.reserve(str.size());
    for (char c : str) {
        res.push_back(static_cast<wchar_t>(c));
    }

    return res;
}


std::wstring watch_model::text(const row_index & row, std::size_t c) const {
    assert(row && "invalid row");
    assert(c < columns_size() && "invalid column index");

    const watch_list_tree_node * node = row_node(row);

    switch (c) {
    case 0:
        return node->name();
    case 1:
        return node->value();
    case 2:
        return node->type();
    case 3: {
        if (node->format_time() == ULONG_MAX) {
            return {};
        }

        std::wostringstream str;
        str << node->format_time() << L" ms";
        return str.str();
    }
    default:
        assert(false && "invalid column index");
    }

    return std::wstring();
}


ro_tree_view_model::color_index watch_model::color(const ro_tree_view_model::row_index & row, std::size_t c) const {
    assert(row && "invalid row");
    assert(c < columns_size() && "invalid column index");

    const watch_list_tree_node * node = row_node(row);

    if (node->is_marked()) {
        return 1;
    } else {
        return 0;
    }
}


const watch_list_tree_node * watch_model::row_node(const row_index & row) const {
    assert(row && "invalid row");
    return reinterpret_cast<const watch_list_tree_node*>(row.ptr());
}


watch_model::row_index watch_model::make_node_index(const watch_list_tree_node * node) const {
    return make_index(const_cast<watch_list_tree_node*>(node));
}


std::size_t watch_model::index_in_parent(const watch_list_tree_node * node) const {
    assert(node != nullptr && "invalid node");

    const watch_list_tree_node * parent = node->parent();
    if (parent == nullptr) {
        // node is root node
        auto res = std::find(std::begin(root_nodes_), std::end(root_nodes_), node);
        assert(res != std::end(root_nodes_) && "invalid root node");
        auto index = res - std::begin(root_nodes_);
        assert(index >= 0 && "invalid root node index");
        return static_cast<std::size_t>(index);
    }

    for (std::size_t i = 0, e = parent->childs_size(); i < e; ++i) {
        if (parent->child_at(i) == node) {
            return i;
        }
    }

    assert(false && "invalid parent node");
    return SIZE_MAX;
}


void watch_model::on_root_node_added(const watch_list_tree_node * node) {
    // emitting before row added signal
    before_added()({}, root_nodes_.size(), root_nodes_.size());

    // adding node to vector of root nodes
    root_nodes_.push_back(node);

    // emitting after row added signal
    after_added()({}, root_nodes_.size() - 1, root_nodes_.size() - 1);
}


void watch_model::on_root_node_removed(const watch_list_tree_node * node) {

    assert(node != nullptr && "invalid node");
    std::size_t idx = index_in_parent(node);

    // emitting before row removed signal
    before_removed()({}, idx, idx);

    // removing node
    root_nodes_.erase(std::begin(root_nodes_) + idx);

    // emitting after row removed signal
    after_removed()({}, idx, idx);
}


void watch_model::on_node_changed(const watch_list_tree_node * node) {
    // emitting row changed signal
    assert(node && "invalid node");
    after_changed()(make_node_index(node));
}


tree_view_expand_state watch_model::get_expand_state(const ro_tree_view_model::row_index & index) const {
    auto * node = row_node(index);
    assert(node != nullptr && "invalid index");
    auto state = node->expand_state();

    return state;
}


void watch_model::set_expand_state(const ro_tree_view_model::row_index & index, tree_view_expand_state state) const {
    CXXDBG_LOG_SCAT_TRACE(dbg, watchmodel) << "set expand state: "
        << index.ptr() << ", " << tree_view_expand_state_to_string(state);

    assert(state != tree_view_expand_state::undefined && "undefined expand state should not be passed here");

    auto * node = row_node(index);

    if (state == tree_view_expand_state::expanded) {
        watch_.expand_node(node);
    } else if (state == tree_view_expand_state::collapsed) {
        watch_.collapse_node(node);
    } else {
        assert(false && "unknown expand state");
    }
}


}
