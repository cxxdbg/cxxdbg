// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file simple_tree_view_model.hpp
/// Contains definition of the simple_tree_view_model class

#pragma once

#include "tree_view_model.hpp"


namespace cxxdbg {


/// Simple implementation of tree view model that stores string values
/// for each node. Used for testing.
class simple_tree_view_model: virtual public tree_view_model {

    /// Model node
    class node {
    public:
        /// Constructs node with specified number of columns
        node(node * prnt, size_t num_columns):
        parent_{prnt} {
            vals_.resize(num_columns);
        }

        /// Returns text for column with specified index
        const std::wstring & text(size_t c) const {
            assert(c < vals_.size() && "invlid column index");
            return vals_[c];
        }

        /// Sets text for column with specified index
        void set_text(size_t c, const std::wstring & txt) {
            assert(c < vals_.size() && "invlid column index");
            vals_[c] = txt;
        }

        /// Returns number of child nodex
        size_t childs_size() const {
            return childs_.size();
        }

        /// Returns child node at specified index
        auto child(size_t idx) const {
            assert(idx < childs_.size()  && "invlid child index");
            return childs_[idx];
        }

        /// Adds child node at specified index
        void insert(size_t idx, const std::shared_ptr<node> & n) {
            assert(idx <= childs_.size() && "invalid child index");
            childs_.insert(childs_.begin() + idx, n);
        }

        /// Removes child node at specified index
        void remove(size_t idx, size_t nchilds) {
            assert(idx + nchilds - 1< childs_.size() && "invalid child index");
            auto start = childs_.begin() + idx;
            auto end = start + nchilds;
            childs_.erase(start, end);
        }

        /// Returns pointer to parent node
        node * parent() const {
            return parent_;
        }

        /// Returns index of child node
        size_t child_index(node * chld) const;

    private:
        node * parent_;                                 ///< Pointer to parent node
        std::vector<std::wstring> vals_;                ///< Column values
        std::vector<std::shared_ptr<node>> childs_;     ///< Child nodes
    };

public:    
    /// Constructs empty tree view model with specified number of columns
    simple_tree_view_model(size_t num_columns, bool editable = true):
        column_names_{num_columns}, editable_{editable}, root_{nullptr, num_columns} {}

    /// Non copyable
    simple_tree_view_model(const simple_tree_view_model &) = delete;

    /// Non movable
    simple_tree_view_model(simple_tree_view_model &&) = delete;

    /// Destructor, destoys object
    virtual ~simple_tree_view_model() = default;

    /// Returns number of columns
    std::size_t columns_size() const override { return column_names_.size(); }

    /// Sets name of column with specified index
    void set_column_name(size_t idx, const std::wstring & nm) {
        assert(idx < columns_size() && "invalid column index");
        column_names_[idx] = nm;
    }

    /// Returns name of column with specified index
    std::wstring column_name(std::size_t index) const override {
        assert(index < columns_size() && "invalid column index");
        return column_names_[index];
    }

    /// Returns number of child rows for row
    std::size_t childs_size(const row_index & row) const override;

    /// Returns child row with specified index
    row_index child(const row_index & row, std::size_t index) const override;

    /// Inserts childs into node with specified index
    void insert(const row_index & row,
                size_t idx,
                const std::vector<std::vector<std::wstring>> & vals);

    /// Removes childs at specified index
    void remove(const row_index & row, size_t idx, size_t nchilds = 1);

    /// Returns index of row in parent
    std::size_t index(const row_index & row) const override;

    /// Returns parent for specified row
    row_index parent(const row_index & row) const override;

    /// Returns text for specified cell
    std::wstring text(const row_index & row, std::size_t c) const override;

    /// Returns true if cell is editable. Returns true if editable flag is set
    bool editable(const row_index & r, std::size_t c) override;

    /// Sets text for specified cell
    void set_text(const row_index & r, std::size_t c, const std::wstring & s) override;

private:
    /// Makes model index from pointer to node
    row_index make_index(node * n) const { return tree_view_model::make_index(n); }

    /// Converts model index to pointer to node
    node * row_node(const row_index & idx) const {
        if (!idx.is_valid()) {
            // root node
            return const_cast<node*>(&root_);
        }

        return static_cast<node*>(idx.ptr());
    }

    std::vector<std::wstring> column_names_;    ///< Names of columns
    bool editable_;                             ///< Is model editable?
    node root_;                                 ///< Root node
};


}


