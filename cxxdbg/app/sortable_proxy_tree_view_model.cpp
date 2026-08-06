// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file sortable_proxy_tree_view_model.cpp
/// Contains implementation of sorted tree view model classes.

#include "sortable_proxy_tree_view_model.hpp"
#include <cassert>


namespace cxxdbg {


sortable_proxy_ro_tree_view_model::sortable_proxy_ro_tree_view_model(ro_tree_view_model & src_model):
src_mdl_{src_model} {
    // building vector of root nodes
    auto nchilds = src_model.childs_size({});
    nodes_.reserve(nchilds);
    for (size_t i = 0; i < nchilds; ++i) {
        nodes_.push_back(src_model.child({}, i));
    }


    // connecting to source model signals

    before_added_con_ = src_model.before_added().connect([this](auto && ... args) {
        this->on_before_added(args...);
    });

    after_added_con_ = src_model.after_added().connect([this](auto && ... args) {
        this->on_after_added(args...);
    });

    before_removed_con_ = src_model.before_removed().connect([this](auto && ... args) {
        this->on_before_removed(args...);
    });

    after_removed_con_ = src_model.after_removed().connect([this](auto && ... args) {
        this->on_after_removed(args...);
    });

    after_changed_con_ = src_model.after_changed().connect([this](auto && ... args) {
        this->on_after_changed(args...);
    });

    before_layout_changed_con_ = src_model.before_layout_changed().connect([this](auto && ... args) {
        this->on_before_layout_changed(args...);
    });

    after_layout_changed_con_ = src_model.after_layout_changed().connect([this](auto && ... args) {
        this->on_after_layout_changed(args...);
    });
}


std::size_t sortable_proxy_ro_tree_view_model::columns_size() const {
    return src_mdl_.columns_size();
}


std::wstring sortable_proxy_ro_tree_view_model::column_name(std::size_t index) const {
    return src_mdl_.column_name(index);
}


std::size_t sortable_proxy_ro_tree_view_model::childs_size(const row_index & row) const {
    if (row.is_valid()) {
        return src_mdl_.childs_size(row);
    }

    return nodes_.size();
}


auto sortable_proxy_ro_tree_view_model::child(const row_index & row, std::size_t index) const -> row_index {
    if (!row.is_valid()) {
        // root node
        assert(index < nodes_.size() && "invalid root node index");
        return nodes_[index];
    }

    return src_mdl_.child(row, index);
}


std::size_t sortable_proxy_ro_tree_view_model::index(const row_index & row) const {
    assert(row.is_valid() && "invalid row index");

    row_index parent = src_mdl_.parent(row);
    if (parent.is_valid()) {
        // not root node, asking source model for index
        return src_mdl_.index(row);
    }

    // calculating index of root node
    auto res = std::find(nodes_.begin(), nodes_.end(), row);
    assert(res != nodes_.end() && "can't find root row in sorted vector");

    return std::distance(nodes_.begin(), res);
}


auto sortable_proxy_ro_tree_view_model::parent(const row_index & row) const -> row_index{
    return src_mdl_.parent(row);
}


std::wstring sortable_proxy_ro_tree_view_model::text(const row_index & row, std::size_t c) const {
    return src_mdl_.text(row, c);
}


auto sortable_proxy_ro_tree_view_model::image(const row_index & row, std::size_t c) const -> image_index {
    return src_mdl_.image(row, c);
}


void sortable_proxy_ro_tree_view_model::sort(size_t column, sort_order order) {

    assert(column < columns_size() && "invalid column index");

    // sending before_layout_changed signal
    before_layout_changed()({});

    // if sorting is set to no then copy order of root nodes from source model
    if (order == sort_order::no) {
        for (size_t i = 0, e = src_mdl_.childs_size({}); i < e; ++i) {
            nodes_[i] = src_mdl_.child({}, i);
        }

    } else {
        order_ = order;
        sort_col_ = column;

        // sorting vector of root nodes
        if (order_ == sort_order::ascending) {
            std::sort(nodes_.begin(), nodes_.end(), [this](const auto & x, const auto & y) {
                return src_mdl_.text(x, sort_col_) < src_mdl_.text(y, sort_col_);
            });
        } else {
            assert(order_ == sort_order::descending && "logic error");
            std::sort(nodes_.begin(), nodes_.end(), [this](const auto & x, const auto & y) {
                return src_mdl_.text(x, sort_col_) > src_mdl_.text(y, sort_col_);
            });
        }
    }

    // sending after_layout_changed signal
    after_layout_changed()({});
}


void sortable_proxy_ro_tree_view_model::on_before_added(const row_index & idx, size_t start, size_t end) {

    if (idx.is_valid()) {
        // not root nodes, don't care about them
        before_added()(idx, start, end);
        return;
    }

    if (order_ == sort_order::no) {
        // sorting is disabled, we can just add childs in same order as in source model

        before_added()(idx, start, end);
        return;
    }

    // we will insert all new nodes one by one in the after_added signal handler.
    // doing nothing here
}


void sortable_proxy_ro_tree_view_model::on_after_added(const row_index & idx, size_t start, size_t end) {
    if (idx.is_valid()) {
        // not root nodes, don't care about them
        after_added()(idx, start, end);
        return;
    }

    assert(start <= nodes_.size() && "invalid add index");
    assert(end >= start && "invalid start/end indexes");

    if (order_ == sort_order::no) {
        // sorting is disabled, we can just add childs in same order as in source model

        // allocating vector of new nodes
        auto new_size = nodes_.size() + end - start + 1;
        assert(new_size == src_mdl_.childs_size({}) &&
               "signals/nodes inconsistency in source model");
        std::vector<row_index> new_nodes;
        new_nodes.reserve(new_size);

        // copying beginning of old nodes
        std::copy(nodes_.begin(), nodes_.begin() + start, std::back_inserter(new_nodes));

        // adding new nodes
        for (size_t i = start; i <= end; ++i) {
            new_nodes.push_back(src_mdl_.child({}, i));
        }

        // copying all remaining nodes
        std::copy(nodes_.begin() + start, nodes_.end(), std::back_inserter(new_nodes));

        // exchanging old/new node vectors
        std::swap(nodes_, new_nodes);

        // sending signal to observers
        after_added()(idx, start, end);

        return;
    }


    // inserting new nodes one by one.
    // N^2 complexity. TODO: try find better solution?
    for (size_t i = start; i <= end; ++i) {
        insert_root_node(src_mdl_.child({}, i));
    }
}


void sortable_proxy_ro_tree_view_model::on_before_removed(const row_index & idx, size_t start, size_t end) {
    if (idx.is_valid()) {
        // not root nodes, don't care about them
        before_removed()(idx, start, end);
        return;
    }

    if (order_ == sort_order::no) {
        // sorting is disabled, we can just remove childs at same indexes as in source model

        before_removed()(idx, start, end);
        return;
    }


    // removing nodes one by one
    // N^2 complexity. TODO: try find better solution?
    for (size_t i = start; i <= end; ++i) {
        auto child_idx = src_mdl_.child({}, i);

        // looking for child index in the sorted vector;
        auto it = std::find(nodes_.begin(), nodes_.end(), child_idx);
        assert(it != nodes_.end() && "can't find root node in sorted vector");
        auto real_idx = std::distance(nodes_.begin(), it);

        // removing node
        before_removed()({}, real_idx, real_idx);
        nodes_.erase(it, it + 1);
        after_removed()({}, real_idx, real_idx);
    }
}


void sortable_proxy_ro_tree_view_model::on_after_removed(const row_index & idx, size_t start, size_t end) {
    if (idx.is_valid()) {
        // not root nodes, don't care about them
        after_removed()(idx, start, end);
        return;
    }

    if (order_ == sort_order::no) {
        // sorting is disabled, we can just remove root nodes at same
        // indexes as in the source model

        assert(end < nodes_.size() && "invalid add index");
        assert(end >= start && "invalid start/end indexes");

        nodes_.erase(nodes_.begin() + start, nodes_.begin() + end + 1);
        after_removed()(idx, start, end);
        return;
    }

    // if sorting enabled then all root nodes was removed in the before_removed handler
    // doing nothing here
}


void sortable_proxy_ro_tree_view_model::on_after_changed(const row_index & idx) {
    after_changed()(idx);
}


void sortable_proxy_ro_tree_view_model::on_before_layout_changed(const row_index & idx) {
    if (idx.is_valid()) {
        // not root nodes, don't care about them
        before_layout_changed()(idx);
        return;
    }

    // changing layout of nodes only if sorting is disabled
    if (order_ != sort_order::no) {
        return;
    }

    before_layout_changed()({});
}


void sortable_proxy_ro_tree_view_model::on_after_layout_changed(const row_index & idx) {
    if (idx.is_valid()) {
        // not root nodes, don't care about them
        after_layout_changed()(idx);
        return;
    }

    // changing layout of nodes only if sorting is disabled
    if (order_ != sort_order::no) {
        return;
    }

    // copying root node indexes from source model
    auto sz = src_mdl_.childs_size({});
    assert(sz == nodes_.size() && "root size inconsistency in source/sroted models");
    for (size_t i = 0; i < sz; ++i) {
        nodes_[i] = src_mdl_.child({}, i);
    }

    // notifying observers
    after_layout_changed()({});
}


size_t sortable_proxy_ro_tree_view_model::calc_insert_index(const row_index & idx) {
    assert(order_ != sort_order::no && "calc_insert_index called with no sorting");

    std::vector<row_index>::iterator res;

    if (order_ == sort_order::ascending) {
        res = std::find_if(nodes_.begin(), nodes_.end(), [this, idx](const auto & item) {
            return src_mdl_.text(idx, sort_col_) < src_mdl_.text(item, sort_col_);
        });
    } else {
        res = std::find_if(nodes_.begin(), nodes_.end(), [this, idx](const auto & item) {
            return src_mdl_.text(idx, sort_col_) > src_mdl_.text(item, sort_col_);
        });
    }

    return std::distance(nodes_.begin(), res);
}


void sortable_proxy_ro_tree_view_model::insert_root_node(const row_index & idx) {
    assert(order_ != sort_order::no && "insert_root_node called with no sorting");

    // calculating index of new node
    auto insert_index = calc_insert_index(idx);

    // inserting new node
    before_added()({}, insert_index, insert_index);
    nodes_.insert(nodes_.begin() + insert_index, idx);
    after_added()({}, insert_index, insert_index);
}


ro_tree_view_model::color_index
sortable_proxy_ro_tree_view_model::color(const ro_tree_view_model::row_index & row, std::size_t c) const {
    return src_mdl_.color(row, c);
}


bool sortable_proxy_ro_tree_view_model::has_expand_state_support() const {
    return src_mdl_.has_expand_state_support();
}


tree_view_expand_state sortable_proxy_ro_tree_view_model::get_expand_state(const tree_view_model_row_index & index) const {
    return src_mdl_.get_expand_state(index);
}


void sortable_proxy_ro_tree_view_model::set_expand_state(const ro_tree_view_model::row_index & index,
                                                         tree_view_expand_state state) const {
    src_mdl_.set_expand_state(index, state);
}


sortable_proxy_tree_view_model::sortable_proxy_tree_view_model(tree_view_model & src_mdl):
sortable_proxy_ro_tree_view_model{src_mdl}, src_mdl_{src_mdl} {
}


bool sortable_proxy_tree_view_model::editable(const row_index & r, std::size_t c) {
    return src_mdl_.editable(r, c);
}


void sortable_proxy_tree_view_model::set_text(const row_index & r, std::size_t c, const std::wstring & s) {
    src_mdl_.set_text(r, c, s);
}


}
