// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file vector_ptr_model.hpp
/// Contains definition of the vector_ptr_model class.

#pragma once

#include "signals.hpp"
#include <cassert>
#include <vector>
#include <boost/concept_check.hpp>
#include <boost/iterator/iterator_adaptor.hpp>


namespace cxxdbg {


/// Vector ptr model concept
template <typename T>
struct VectorPtrModel {
    using size_type = typename T::size_type;
    using value_type = typename T::value_type;

public:
    BOOST_CONCEPT_USAGE(VectorPtrModel) {
        // model size
        size_type sz = cmodel_.size();

        // acessing element by index
        const value_type * item = cmodel_.at(idx_);
        item = cmodel_[idx_];

        // inerting item at index
        model_.insert(idx_, std::move(new_val_));

        // adding item at the end
        model_.push_back(std::move(new_val_));

        // iterating over model
        for (const value_type * item : cmodel_) {
        }

        // signals
        con_ = model_.before_added().connect([](size_type, size_type) {});
        con_ = model_.after_added().connect([](size_type, size_type) {});
        con_ = model_.before_removed().connect([](size_type, size_type) {});
        con_ = model_.after_removed().connect([](size_type, size_type) {});
        con_ = model_.before_changed().connect([](size_type) {});
        con_ = model_.after_changed().connect([](size_type) {});
        con_ = model_.list_changed().connect([] {});
    }

private:
    T model_;
    const T cmodel_;
    signal_connection con_;
    std::unique_ptr<value_type> new_val_;
    size_type idx_;
};


/// Generic vector model that holds vector of unique pointers to items and send
/// signals when items added/removed/changed
template <typename T>
class vector_ptr_model {
    using vector_type = std::vector<std::unique_ptr<T>>;
    using vector_const_iterator = typename vector_type::const_iterator;

public:
    /// Type of size and index in model
    using size_type = size_t;

    /// Type of value
    using value_type = T;

    /// Const iterator over elements
    class const_iterator;

    /// Helper class for modifying model items. Calls before_changed/after_changed
    /// in constructor and destructor
    class item_changer {
    public:
        /// Constructs invalid item changer
        item_changer() = default;

        /// Constructs changer for item with specified index. Emits before_changed signal
        item_changer(vector_ptr_model<T> * mdl, size_t idx):
        mdl_{mdl}, idx_{idx} {
            mdl_->before_changed()(idx_);
        }

        /// Destroys changer and emits after_changed signal if changer is valid
        ~item_changer() {
            if (idx_ != SIZE_MAX) {
                assert(mdl_ != nullptr && "invalid pointer to model");
                mdl_->after_changed()(idx_);
            }
        }

        /// Deleted copy constructor
        item_changer(const item_changer &) = delete;

        /// Move constructor, makes source changer invalid
        item_changer(item_changer && src):
        item_changer(src.mdl_, src.idx_) {
            src.mdl_ = nullptr;
            src.idx_ = SIZE_MAX;
        }

        /// Deleted copy assignment operator
        item_changer & operator=(const item_changer &) = delete;

        /// Move assignment operator, makes right side changer invalid.
        /// This changer must be invalid
        item_changer & operator=(item_changer && src) {
            assert(idx_ == SIZE_MAX && "can't assign to valid changer");
            mdl_ = src.mdl_;
            idx_ = src.idx_;
            src.mdl_ = nullptr;
            src.idx_ = SIZE_MAX;
        }

        /// Returns pointer to item value
        T * get() const {
            assert(idx_ < mdl_->size() && "invalid item index");
            return mdl_->items_[idx_].get();
        }

        /// Returns pointer to item value
        T * operator->() const {
            return get();
        }

        /// Emits after_changed signal and makes this changer invalid
        void commit() {
            assert(mdl_ != nullptr && idx_ != SIZE_MAX && "invalid changer");
            mdl_->after_changed()(idx_);
            mdl_ = nullptr;
            idx_ = SIZE_MAX;
        }

    private:
        vector_ptr_model<T> * mdl_ = nullptr;
        size_t idx_ = SIZE_MAX;
    };

    /// Constructs empty model
    vector_ptr_model() = default;

    /// Destroys model
    ~vector_ptr_model() = default;

    /// Rerurns count of items in model
    size_type size() const  { return items_.size(); }

    /// Returns true if model is empty
    bool empty() const { return items_.empty(); }

    /// Returns const pointer to item at specified index with checks
    const T * at(size_type idx) const { return items_.at(idx).get(); }

    /// Returns const pointer to item at specified index without checks
    const T * operator[](size_type idx) const {
        assert(idx < size() && "invalid item index");
        return items_[idx].get();
    }

    /// Returns const iterator pointing to the first item
    auto begin() const { return const_iterator{items_.begin()}; }

    /// Returns const iterator pointing to one past end item
    auto end() const { return const_iterator{items_.end()}; }

    /// Inserts item at the end of model
    void push_back(std::unique_ptr<T> && val) {
        before_added()(size(), size());
        items_.push_back(std::move(val));
        after_added()(size(), size());
    }

    /// Inserts item at specified index
    void insert(size_type idx, std::unique_ptr<T> && val) {
        assert(idx <= size() && "invalid item index");
        before_added()(idx, idx);
        items_.insert(items_.begin() + idx, std::move(val));
        after_added()(idx, idx);
    }

    /// Removes item at specified index
    void erase(size_type idx) {
        assert(idx < size() && "invalid item index");
        before_removed()(idx, idx);
        items_.erase(items_.begin() + idx);
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

    /// Starts changing of item with specified index. Emits before_changed signal
    /// and returns helper class that will call after_changed in destructor
    item_changer begin_change(size_t idx) {
        assert(idx < size() && "invalid item index");
        return item_changer{this, idx};
    }

    /// The signal is emitted before items added
    CXXDBG_DEFINE_SIGNALX(before_added, void (size_type, size_type))

    /// The signal is emitted after items added
    CXXDBG_DEFINE_SIGNALX(after_added, void (size_type, size_type))

    /// The signal is emitted before items removed
    CXXDBG_DEFINE_SIGNALX(before_removed, void (size_type, size_type))

    /// The signal is emitted after items removed
    CXXDBG_DEFINE_SIGNALX(after_removed, void (size_type, size_type))

    /// The signal is emitted before item is changed
    CXXDBG_DEFINE_SIGNALX(before_changed, void (size_type))

    /// The signal is emitted after item is changed
    CXXDBG_DEFINE_SIGNALX(after_changed, void (size_type))

    /// The signal is emitted after list changed (items added/removed/changed)
    CXXDBG_DEFINE_SIGNALX(list_changed, void())

private:
    vector_type items_;                 ///< Vector of items
};


template <typename T>
class vector_ptr_model<T>::const_iterator: public boost::iterator_adaptor <
    vector_ptr_model<T>::const_iterator,
    vector_const_iterator,
    const T *,
    boost::random_access_traversal_tag,
    const T *
> {
public:
    explicit const_iterator(const vector_const_iterator & it): it_{it} {}

    /// Dereferences iterator
    const T * dereference() const {
        return it_->get();
    }

private:
    vector_const_iterator it_;
};


BOOST_CONCEPT_ASSERT((VectorPtrModel<vector_ptr_model<int>>));


}


