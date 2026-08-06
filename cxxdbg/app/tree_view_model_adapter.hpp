// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file tree_view_model_adapter.hpp
/// Contains definition of the tree_view_model_adapter template class.

#pragma once

#include "tree_model.hpp"
#include "tree_view_model.hpp"
#include <boost/concept_check.hpp>


namespace cxxdbg {


/// Node adapter concept for tree view adapter
template <typename Adapter, typename Value>
struct TreeViewModelNodeAdapter {
    using value_type = Value;

public:
    TreeViewModelNodeAdapter() = delete;

    BOOST_CONCEPT_USAGE(TreeViewModelNodeAdapter) {
        // number of columns
        sz_ = adapter_.columns_size();

        // getting column name
        str_ = adapter_.column_name(idx_);

        // text for node
        str_ = adapter_.text(node_, idx_);

        // image for node
        img_idx_ = adapter_.image(node_, idx_);

        // color for node
        col_idx_ = adapter_.color(node_, idx_);
    }

private:
    const value_type node_;
    Adapter adapter_;
    std::wstring str_;
    size_t sz_;
    size_t idx_;
    ro_tree_view_model::image_index img_idx_;
    ro_tree_view_model::color_index col_idx_;
};


/// Adapter for tree model that implements ro_tree_view_model interface
template <typename BaseModel, typename NodeAdapter>
class ro_tree_view_model_adapter: virtual public ro_tree_view_model {
    BOOST_CONCEPT_ASSERT((TreeModel<BaseModel>));

    using value_type = typename BaseModel::value_type;

    BOOST_CONCEPT_ASSERT((TreeViewModelNodeAdapter<NodeAdapter, value_type>));

public:
    /// Constructs adapter model with specified reference to base model
    /// and node adapter value
    ro_tree_view_model_adapter(const BaseModel & base,
                               const NodeAdapter & nadapter = {}):
    base_{base}, node_adapter_{nadapter} {
        before_added_con_ = base_.before_added.connect(
        [this](auto node, size_t first, size_t last) {
            before_added()(node_to_row(node), first, last);
        });

        after_added_con_ = base_.after_added.connect(
        [this](auto node, size_t first, size_t last) {
            after_added()(node_to_row(node), first, last);
        });

        before_removed_con_ = base_.before_removed.connect(
        [this](auto node, size_t first, size_t last) {
            before_removed()(node_to_row(node), first, last);
        });

        after_removed_con_ = base_.after_removed.connect(
        [this](auto node, size_t first, size_t last) {
            after_removed()(node_to_row(node), first, last);
        });

        before_changed_con_ = base_.before_changed.connect([this](auto) {
            // TODO: ro_tree_view_model does not have before_changed signal
        });

        after_changed_con_ = base_.after_changed.connect([this](auto node) {
            after_changed()(node_to_row(node));
        });
    }

    /// Destoys adapter
    ~ro_tree_view_model_adapter() override = default;

    /// Returns number of columns
    std::size_t columns_size() const override {
        return node_adapter_.columns_size();
    }

    /// Returns name of column with specified index
    std::wstring column_name(std::size_t index) const override {
        return node_adapter_.column_name(index);
    }

    /// Returns number of child rows for row
    std::size_t childs_size(const row_index & row) const override {
        return base_.childs_size(row_to_node(row));
    }

    /// Returns child row with specified index
    row_index child(const row_index & row, std::size_t index) const override {
        return node_to_row(base_.child(row_to_node(row), index));
    }

    /// Returns index of row in parent
    std::size_t index(const row_index & row) const override {
        return base_.index(row_to_node(row));
    }

    /// Returns parent for specified row
    row_index parent(const row_index & row) const override {
        return node_to_row(base_.parent(row_to_node(row)));
    }

    /// Returns text for specified cell
    std::wstring text(const row_index & row, std::size_t c) const override {
        return node_adapter_.text(row_to_node(row), c);
    }

    /// Returns image index for specified cell. Default implementation returns 0.
    image_index image(const row_index & row, std::size_t c) const override {
        return node_adapter_.image(row_to_node(row), c);
    }

    /// Returns color index for specified cell. Default implementation returns 0.
    color_index color(const row_index & row, std::size_t c) const override {
        return node_adapter_.color(row_to_node(row), c);
    }

    /// Converts row index to node
    static value_type row_to_node(const row_index & row) {
        return reinterpret_cast<value_type>(row.ptr());
    }

    /// Converts node to row index
    static row_index node_to_row(const value_type & n) {
        return make_index(const_cast<void*>(reinterpret_cast<const void*>(n)));
    }

    /// Returns reference to base model
    const BaseModel & base() const { return base_; }

private:

    const BaseModel & base_;        ///< Reference to base model
    NodeAdapter node_adapter_;      ///< Node adapter

    scoped_signal_connection before_added_con_;
    scoped_signal_connection after_added_con_;
    scoped_signal_connection before_removed_con_;
    scoped_signal_connection after_removed_con_;
    scoped_signal_connection before_changed_con_;
    scoped_signal_connection after_changed_con_;

};


}


