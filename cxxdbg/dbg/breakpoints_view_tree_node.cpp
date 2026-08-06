// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

//
// Created by yzarudny on 08.12.17.
//

#include "breakpoints_view_tree_node.hpp"
#include "breakpoint_site.hpp"
#include "code_breakpoint.hpp"
#include "breakpoint_location.hpp"
#include "watchpoint.hpp"

#include "cxxdbg/util/convert.hpp"

#include <assert.h>
#include <cxxdbg/util/print.hpp>

#include <ranges.hpp>


namespace cxxdbg::dbg {

using namespace util;


class breakpoint_node_info: public breakpoints_view_tree_node::node_info {
public:
    explicit breakpoint_node_info(const code_breakpoint * bp): bp_(bp) {
    }

    ~breakpoint_node_info() override = default;

    std::wstring name() const override {
        std::string bpName;

        if (bp_->locations_size() == 1) {
            auto locs = bp_->locations();
            auto * bpLoc = *std::ranges::begin(locs);
            bpName = bp_->name() + " (" + bpLoc->pos().src_pos_str() + ")";
        } else {
            bpName = bp_->name();
        }

        return convert::to_wstring(bpName);
    }

    std::wstring condition() const override {
        return convert::to_wstring(bp_->condition());
    }

    std::wstring hit_count() const override {
        return convert::to_wstring(bp_->hit_count_str());
    }

    std::wstring function() const override {
        if (bp_->locations_size() == 1) {
            auto locs = bp_->locations();
            auto * bpLoc = *std::ranges::begin(locs);
            return convert::to_wstring(bpLoc->pos().func_name());
        }

        return {};
    }

    std::wstring address() const override {
        if (bp_->locations_size() == 1) {
            auto locs = bp_->locations();
            auto * bpLoc = *std::ranges::begin(locs);
            return convert::to_wstring(bpLoc->pos().addr_str());
        }

        return {};
    }

    std::size_t image_index() const override {
        if (bp_->enabled() && bp_->locations_size() > 0) {
            return 1;
        } else {
            return 2;
        }
    }

    const breakpoint_site * get_site() const override {
        return dynamic_cast<const breakpoint_site *>(bp_);
    }

    breakpoints_view_tree_node::node_type type() const override {
        return breakpoints_view_tree_node::type_code_breakpoint;
    }

private:
    const code_breakpoint * bp_;
};

class location_node_info: public breakpoints_view_tree_node::node_info {
public:
    explicit location_node_info(const breakpoint_location * loc): loc_(loc) {
        assert(loc_ != nullptr && "");
    }

    ~location_node_info() override = default;

    std::wstring name() const override {
        return convert::to_wstring(loc_->pos().src_pos_str());
    }

    std::wstring condition() const override {
        return {};
    }

    std::wstring hit_count() const override {
        return {};
    }

    std::wstring function() const override {
        return convert::to_wstring(loc_->pos().func_name());
    }

    std::wstring address() const override {
        return convert::to_wstring(loc_->pos().addr_str());
    }

    std::size_t image_index() const override {
        return 0;
    }

    const breakpoint_site * get_site() const override {
        return dynamic_cast<const breakpoint_site *>(loc_);
    }

    breakpoints_view_tree_node::node_type type() const override {
        return breakpoints_view_tree_node::type_location;
    }

private:
    const breakpoint_location * loc_;
};


class watchpoint_node_info: public breakpoints_view_tree_node::node_info {
public:
    explicit watchpoint_node_info(const watchpoint * wp): wp_(wp) {
        assert(wp_ && "no wp");
    }

    ~watchpoint_node_info() override = default;

    std::wstring name() const override {
        auto read_write_attribute = [&]() -> std::wstring {
            bool is_read = wp_->is_read();
            bool is_write = wp_->is_write();

            if (is_read && is_write) {
                return L"read/write";
            } else if (is_write) {
                return L"write";
            } else if (is_read) {
                return L"read";
            }

            assert(false && "shouldn't get here");
            return {};
        };

        std::wstring bpName = L"When <" + convert::to_wstring(wp_->name()) + L"> " + read_write_attribute();

        return bpName;
    }

    std::wstring condition() const override {
        return convert::to_wstring(wp_->condition());
    }

    std::wstring hit_count() const override {
        return convert::to_wstring(wp_->hit_count_str());
    }

    std::wstring function() const override {
        return {};
    }

    std::wstring address() const override {
        auto addr = wp_->address();
        if (!addr) {
            return {};
        }

        std::wostringstream str;
        util::print_hex(str, addr, 8, 4);
        return str.str();
    }

    std::size_t image_index() const override {
        if (wp_->enabled()) {
            return 1;
        } else {
            return 2;
        }
    }

    const breakpoint_site * get_site() const override {
        return dynamic_cast<const breakpoint_site *>(wp_);
    }

    breakpoints_view_tree_node::node_type type() const override {
        return breakpoints_view_tree_node::type_watchpoint;
    }

private:
    const watchpoint * wp_;
};


breakpoints_view_tree_node::tree_node_ptr breakpoints_view_tree_node::create_node(const breakpoint * bp) {
    if (const auto * wp = dynamic_cast<const watchpoint *>(bp)) {
        return make_watchpoint_node(wp);
    } else if (const auto * cbp = dynamic_cast<const code_breakpoint *>(bp)) {
        return make_breakpoint_node(cbp);
    } else {
        assert(false && "unknown breakpoint subtype");
        return {};
    }
}

breakpoints_view_tree_node::breakpoints_view_tree_node(breakpoints_view_tree_node::info_ptr info,
                                                       breakpoints_view_tree_node::tree_node * parent):
        info_{info}, parent_{parent} {
}

breakpoints_view_tree_node::tree_node * breakpoints_view_tree_node::child_at(size_t i) const {
    assert(i < children_.size());
    return children_.at(i).get();
}

std::size_t breakpoints_view_tree_node::parent_index() const {
    assert(parent_ != nullptr && "cannot get parent index when parent is null");

    auto & nodes = parent_->children_;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        auto * node = nodes[i].get();
        if (node == this) {
            return i;
        }
    }

    assert(false && "no such child in parent");
    return 0;
}

breakpoints_view_tree_node::node_info * breakpoints_view_tree_node::get_info() const {
    return info_.get();
}

void breakpoints_view_tree_node::remove_all_children() {
    children_.clear();
}

void breakpoints_view_tree_node::add_child(breakpoints_view_tree_node::tree_node_ptr node) {
    children_.push_back(node);
}

breakpoints_view_tree_node::tree_node_ptr
breakpoints_view_tree_node::make_breakpoint_node(const code_breakpoint * bp) {
    assert(bp != nullptr && "breakpoint must not be nullptr");

    auto info = std::make_shared<breakpoint_node_info>(bp);
    tree_node_ptr node = std::make_shared<tree_node>(info);

    for (size_t i = 0; i < bp->children_size(); ++i) {
        auto * child = bp->child_at(i);
        auto * loc = dynamic_cast<const breakpoint_location *>(child);
        assert(loc != nullptr && "children is not location type");

        node->add_location(loc);
    }

    return node;
}

breakpoints_view_tree_node::tree_node_ptr
breakpoints_view_tree_node::make_watchpoint_node(const watchpoint * wp) {
    assert(wp != nullptr && "watchpoint must not be nullptr");

    auto info = std::make_shared<watchpoint_node_info>(wp);
    auto node = std::make_shared<tree_node>(info);

    return node;
}

breakpoints_view_tree_node::tree_node_ptr
breakpoints_view_tree_node::add_location(const breakpoint_location * loc) {
    assert(loc != nullptr && "location must not be nullptr");
    assert(info_->type() == type_code_breakpoint && "only code_breakpoint type can have locations");

    auto info = std::make_shared<location_node_info>(loc);
    auto node = std::make_shared<tree_node>(info, this);

    add_child(node);

    return node;
}


}
