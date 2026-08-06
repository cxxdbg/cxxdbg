// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file vector_model.hpp
/// Contains definition of the vector_model class.

#pragma once

#include "signals.hpp"
#include <cassert>
#include <vector>


namespace cxxdbg {


/// Generic vector model that holds vector of items and send signals when
/// items added/removed/changed
template <typename T>
class vector_model {
public:
    /// Type of size and index in model
    using size_type = size_t;

    /// Constructs empty model
    vector_model() = default;

    /// Destroys model
    ~vector_model() = default;

    /// Rerurns count of items in model
    size_type size() const  { return items_.size(); }

    /// Returns true if model is empty
    bool empty() const { return items_.empty(); }

    /// Returns const reference to item at specified index with checks
    const T & at(size_type idx) const { return items_.at(idx); }

    /// Returns const reference to item at specified index without checks
    const T & operator[](size_type idx) const {
        assert(idx < size() && "invalid item index");
        return items_[idx];
    }

    /// Returns const iterator pointing to the first item
    auto begin() const { return items_.begin(); }

    /// Returns const iterator pointing to one past end item
    auto end() const { return items_.end(); }

    /// Inserts item at the end of model
    void push_back(const T & val) {
        before_added()(size(), size());
        items_.push_back(val);
        after_added()(size(), size());
    }

    /// Inserts multiple items at specified index
    template <typename InputIt>
    void insert(size_type idx, InputIt first, InputIt last) {
        assert(idx <= size() && "invalid item index");

        auto count = std::distance(first, last);
        if (count == 0) {
            return;
        }

        before_added()(idx, idx + count - 1);
        items_.insert(items_.begin() + idx, first, last);
        after_added()(idx, idx + count - 1);
    }

    /// Inserts item at specified index
    void insert(size_type idx, const T & val) {
        insert(idx, &val, &val + 1);
    }

    /// Removes item at specified index
    void erase(size_type idx) {
        assert(idx < size() && "invalid item index");
        before_removed()(idx, idx);
        items_.erase(idx);
        after_removed()(idx, idx);
    }

    /// Removes all items from model
    void clear() {
        if (empty()) {
            return;
        }

        before_removed()(0, size() - 1);
        items_.clear();
        after_removed()(0, size() - 1);
    }

    /// Assigns value to element at specified index.
    /// TODO: implement non-const iterators
    void set(size_t idx, const T & val) {
        assert(idx < size() && "invalid item index");
        items_[idx] = val;
        changed()(idx);
    }

    /// The signal is emitted before items added
    CXXDBG_DEFINE_SIGNALX(before_added, void (size_type, size_type))

    /// The signal is emitted after items added
    CXXDBG_DEFINE_SIGNALX(after_added, void (size_type, size_type))

    /// The signal is emitted before items removed
    CXXDBG_DEFINE_SIGNALX(before_removed, void (size_type, size_type))

    /// The signal is emitted after items removed
    CXXDBG_DEFINE_SIGNALX(after_removed, void (size_type, size_type))

    /// The signal is emitted after item is changed
    CXXDBG_DEFINE_SIGNALX(changed, void (size_type))

private:
    std::vector<T> items_;          ///< Vector of items
};


}


