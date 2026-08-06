// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

//
// Created by yzarudny on 09.10.17.
//

#include "breakpoints_view_model.hpp"
#include "breakpoint_list.hpp"
#include "code_breakpoint.hpp"
#include "watchpoint.hpp"


namespace cxxdbg::dbg {


// model implementation
breakpoints_view_model::breakpoints_view_model(breakpoint_list & bplist):
bplist_{bplist} {
    // connect breakpoints view model

    breakpoint_added_connection_ = bplist_.added().connect([this](auto && bp) {
        on_breakpoint_added(bp);
    });

    breakpoint_removed_connection_ = bplist_.removed().connect([this](auto && bp) {
        on_breakpoint_removed(bp);
    });

    breakpoint_changed_connection_ = bplist_.changed().connect([this](auto && bp) {
        on_breakpoint_updated(bp);
    });
}


std::size_t breakpoints_view_model::columns_size() const {
    return 5;
}


std::wstring breakpoints_view_model::column_name(std::size_t index) const {
    switch (index) {
        case 0: return L"Name";
        case 1: return L"Condition";
        case 2: return L"Hit count";
        case 3: return L"Function";
        case 4: return L"Address";
        default:
            return {};
    }
}


std::size_t breakpoints_view_model::childs_size(const ro_tree_view_model::row_index & row) const {
    auto *  node = make_tree_node(row);
    if (nullptr == node) {
        return root_nodes_.size();
    }

    std::size_t size = node->children_size();

    if (size == 1) {
        return 0;
    }

    return size;
}


ro_tree_view_model::row_index
breakpoints_view_model::child(const ro_tree_view_model::row_index & row, std::size_t index) const {
    auto *  node = make_tree_node(row);
    if (nullptr != node) {
        assert(index < node->children_size() && "child node index out of bound");
        auto * child = node->child_at(index);
        return make_index(child);
    }

    assert(index < root_nodes_.size() && "root node index out of bound");
    auto * child = root_nodes_[index].get();

    return make_index(child);

}


std::size_t breakpoints_view_model::index(const ro_tree_view_model::row_index & row) const {
    assert(row.is_valid() && "invalid row index");
    auto *  node = make_tree_node(row);

    // breakpoint node
    if (node->parent() != nullptr)
        return node->parent_index();

    // location node
    auto & nodes = root_nodes_;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        auto * root_node = nodes[i].get();
        if (root_node == node) {
            return i;
        }
    }

    assert(false && "no such node in the three");

    return 0;
}


ro_tree_view_model::row_index breakpoints_view_model::parent(const ro_tree_view_model::row_index & row) const {
    assert(row.is_valid() && "invalid row index");
    auto *  node = make_tree_node(row);
    auto * parent = node->parent();

    return make_index(parent);
}


std::wstring breakpoints_view_model::text(const ro_tree_view_model::row_index & row, std::size_t c) const {
    assert(row.is_valid() && "invalid row index");
    auto * node = make_tree_node(row);

    auto * info = node->get_info();

    std::wstring value;

    switch (c) {
        case 0:
            value = info->name();
            break;
        case 1:
            value = info->condition();
            break;
        case 2:
            value = info->hit_count();
            break;
        case 3:
            value = info->function();
            break;
        case 4:
            value = info->address();
            break;
        default:
            assert(false && "column index out of range");
    }

    return value;
}


ro_tree_view_model::image_index
breakpoints_view_model::image(const ro_tree_view_model::row_index & row, std::size_t c) const {
    if (c == 0) {
        auto *  node = make_tree_node(row);
        auto * info = node->get_info();

        return info->image_index();
    }

    return 0;
}


ro_tree_view_model::color_index
breakpoints_view_model::color(const ro_tree_view_model::row_index & row, std::size_t c) const {
    return ro_tree_view_model::color(row, c);   // returns 0 means default color
}


breakpoints_view_model::tree_node *
breakpoints_view_model::make_tree_node(const ro_tree_view_model::row_index & row) {
    return reinterpret_cast<tree_node *>(row.ptr());
}


ro_tree_view_model::row_index breakpoints_view_model::make_index(breakpoints_view_model::tree_node * node) {
    return ro_tree_view_model::make_index(static_cast<void*>(node));
}


const breakpoint_location *
breakpoints_view_model::get_single_location(const ro_tree_view_model::row_index & row) const {
    if (!row.is_valid())
        return nullptr;

    auto * node = make_tree_node(row);
    auto * info = node->get_info();
    assert(info != nullptr && "no nodeinfo in node");

    auto * site = info->get_site();
    assert(site != nullptr && "no site in node info");

    return site->get_single_location();
}


const breakpoint_site *
breakpoints_view_model::get_breakpoint_site(const ro_tree_view_model::row_index & row) const {
    if (!row.is_valid()) {
        return nullptr;
    }

    auto * node = make_tree_node(row);
    auto * info = node->get_info();
    assert(info != nullptr && "no nodeinfo in node");

    auto * site = info->get_site();
    assert(site != nullptr && "no site in node info");
    return site;
}


const breakpoint * breakpoints_view_model::get_breakpoint(const ro_tree_view_model::row_index & row) const {
    return dynamic_cast<const breakpoint*>(get_breakpoint_site(row));
}


void breakpoints_view_model::on_breakpoint_added(const breakpoint * bp) {
    std::size_t index = root_nodes_.size();
    before_added()(make_index(nullptr), index, index);

    tree_node::tree_node_ptr new_node = tree_node::create_node(bp);

    root_nodes_.push_back(new_node);
    root_map_.insert(std::make_pair(bp, new_node.get()));

    after_added()(make_index(nullptr), index, index);
}


void breakpoints_view_model::on_breakpoint_updated(const breakpoint * bp) {
    assert(bp != nullptr && "bp must not be nullptr");
    auto * site = dynamic_cast<const breakpoint_site *>(bp);
    assert(site != nullptr && "breakpoints must implement breakpoint_site interface");

    auto it = root_map_.find(bp);
    assert(it != std::end(root_map_));
    auto * node = it->second;
    auto index = make_index(node);

    auto ch_size = node->children_size();

    if (ch_size > 1) {
        before_removed()(index, 0, ch_size - 1);
    }

    node->remove_all_children();

    if (ch_size > 1) {
        after_removed()(index, 0, ch_size - 1);
    }

    auto s_size = site->children_size();

    if (s_size > 1) {
        before_added()(index, 0, s_size - 1);
    }

    // no need to use code_breakpoint interface, breakpoint_site is enough
    for (size_t i = 0; i < s_size; ++i) {
        const auto * child = site->child_at(i);
        const auto * location = dynamic_cast<const breakpoint_location *>(child);
        node->add_location(location);
    }

    if(s_size > 1) {
        after_added()(index, 0, s_size - 1);
    }

    after_changed()(make_index(node));
}


void breakpoints_view_model::on_breakpoint_removed(const breakpoint * bp) {
    auto * site = dynamic_cast<const breakpoint_site *>(bp);
    assert(site != nullptr && "breakpoints must implement breakpoint_site interface");

    auto it = root_map_.find(bp);
    assert(it != std::end(root_map_));

    auto * node = it->second;
    std::size_t index = 0;
    for (std::size_t i = 0; i < root_nodes_.size(); ++i) {
        if (root_nodes_[i].get() == node) {
            index = i;
            break;
        }
    }

    before_removed()(make_index(nullptr), index, index);

    root_nodes_.erase(std::begin(root_nodes_) + index);
    root_map_.erase(it);

    after_removed()(make_index(nullptr), index, index);
}


}
