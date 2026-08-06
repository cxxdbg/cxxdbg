// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file vector_ptr_tree_model.hpp
/// Contains definition of the vector_ptr_tree_model class.

#pragma once

#include "tree_model.hpp"
#include "vector_ptr_model.hpp"
#include <boost/concept_check.hpp>


namespace cxxdbg {


/// Vector model tree view converter converter concept
template <typename Value, typename Converter>
struct VectorPtrTreeModelConverter {
public:
    BOOST_CONCEPT_ASSERT((boost::CopyConstructible<Converter>));

    BOOST_CONCEPT_USAGE(VectorPtrTreeModelConverter) {
        // number of childs for noew
        sz_ = conv_.childs_size(val_ptr_);

        // child at specified index
        val_ptr_ = conv_.child(val_ptr_, idx_);

        // getting parent of item
        val_ptr_ = conv_.parent(val_ptr_);

        // getting index of item in parent
        idx_ = conv_.index(val_ptr_);
    }

private:
    const Value * val_ptr_;
    Converter conv_;
    size_t idx_;
    size_t sz_;
};


/// Converter from vector ptr model to tree model that displays
/// only root items from vector model with no childs
template <typename Value>
struct flat_vector_ptr_tree_model_converter {
    using value_type = Value;

    /// Always return 0 (items don't have childs)
    size_t childs_size(const value_type *) const {
        return 0;
    }

    /// Returns child of item. Should never be called.
    const value_type * child(const value_type *, size_t) const {
        assert(false && "should never be called");
        return nullptr;
    }

    /// Always return nullptr (all items are in root)
    const value_type * parent(const value_type *) const {
        return nullptr;
    }

    /// Returns index of item in parent. Should never be called.
    size_t index(const value_type *) const {
        assert(false && "should never be called");
        return SIZE_MAX;
    }
};


/// Converter from vector ptr model to tree model that redirects all calls
/// to value_type nodes
template <typename Value>
struct intrusive_vector_ptr_tree_model_converter {
    using value_type = Value;

    /// Returns number of childs for item
    size_t childs_size(const value_type * item) const {
        return item->childs_size();
    }

    /// Returns pointer to child of item with specified index
    const value_type * child(const value_type * item, size_t idx) const {
        return item->child(idx);
    }

    /// Returns parent of item
    const value_type * parent(const value_type * item) const {
        return item->parent();
    }

    /// Returns index of item in parent. Searches for passed item in parent list
    /// of childs (N complexity).
    size_t index(const value_type * item) const {
        auto p = parent(item);
        assert(p != nullptr && "parent should not be null here");
        for (size_t i = 0, e = childs_size(p); i < e; ++i) {
            if (child(p, i) == item) {
                return i;
            }
        }

        assert(false && "can't find item in parent");
        return SIZE_MAX;
    }
};


/// Adapter for vector ptr model that satisfies tree model concept and
/// layouts items from base model as tree
template <typename Source,
          typename Value = typename Source::value_type,
          typename Converter = flat_vector_ptr_tree_model_converter<Value>>
class vector_ptr_tree_model {
public:
    BOOST_CONCEPT_ASSERT((VectorPtrModel<Source>));
    BOOST_CONCEPT_ASSERT((VectorPtrTreeModelConverter<Value, Converter>));

    /// Type of base model
    using base_model = Source;

    /// Value type
    using value_type = const Value *;

    /// Size type
    using size_type = typename base_model::size_type;

    /// Signal type
    template <typename Signature>
    using signal = boost::signals2::signal<Signature>;

    /// Constructs wrapper with specified reference to base model. Creates
    /// converter via default constructor
    vector_ptr_tree_model(base_model & base):
    vector_ptr_tree_model(base, Converter{}) {
    }

    /// Constructs wrapper with specified reference to base model and
    /// converter value
    vector_ptr_tree_model(base_model & base, const Converter & c):
    base_{base}, conv_{c} {
        before_added_con_ = base_.before_added().connect(
        [this](size_type first, size_type last) {
            before_added(nullptr, first, last);
        });

        after_added_con_ = base_.after_added().connect(
        [this](size_type first, size_type last) {
            after_added(nullptr, first, last);
        });

        before_removed_con_ = base_.before_removed().connect(
        [this](size_type first, size_type last) {
            before_removed(nullptr, first, last);
        });

        after_removed_con_ = base_.after_removed().connect(
        [this](size_type first, size_type last) {
            after_removed(nullptr, first, last);
        });

        before_changed_con_ = base.before_changed().connect([this](size_type idx) {
            assert(update_item_ == nullptr && "node update is already in progress");

            auto item = base_[idx];

            // if item has childs then simulating removal of all childs
            auto nchilds = childs_size(item);
            if (nchilds != 0) {
                // notifying clients about removing all childs from item being updated
                // after this call childs_size for item will still return original
                // number of childs
                before_removed(item, 0, nchilds - 1);

                // saving item as item being currently updated. After this, childs_size
                // will return 0 for item
                update_item_ = item;

                // notifying clients about finishing of childs removal
                after_removed(item, 0, nchilds - 1);
            }

            // notifying clients about starting item change
            before_changed(item);
        });

        after_changed_con_ = base.after_changed().connect([this](size_type idx) {
            auto item = base_[idx];

            assert(update_item_ == item && "invalid update item");

            // notifying clients about finishing of item change
            after_changed(item);

            // if item has childs then simulating adding of new childs.
            // NOTE: we can't use childs_size for item because it will return 0 here
            auto nchilds = conv_.childs_size(item);
            if (nchilds != 0) {
                // notifying client about adding new childs for item. After this call,
                // childs_size will still return 0 for item
                before_added(item, 0, nchilds - 1);

                // removing item being currently updating. After this, childs_size for
                // item will return real number of childs
                update_item_ = nullptr;

                // notifying clients about finishing of adding new childs
                after_added(item, 0, nchilds - 1);
            }
        });

    }

    /// Destroys wrapper
    ~vector_ptr_tree_model() = default;

    /// Returns number of child rows for specified item
    size_t childs_size(const value_type & item) const {
        if (item == nullptr) {
            // number of root items == base model size
            return base_.size();
        }

        // if item is node being currently updated then we should say that
        // it has no childs
        if (item == update_item_) {
            return 0;
        }

        // getting number of childs from converter
        return conv_.childs_size(item);
    }

    /// Returns child of item
    value_type child(const value_type & item, size_t idx) const {
        if (item == nullptr) {
            // root item
            return base_[idx];
        }

        // child item, asking converter to get pointer to child item
        assert(idx < childs_size(item) && "invalid child index");
        return conv_.child(item, idx);
    }

    /// Returns parent of specified item or nullptr if item does not have parent
    value_type parent(const value_type & item) const {
        return conv_.parent(item);
    }

    /// Returns index of item in parent
    size_t index(const value_type & item) const {
        if (parent(item) == nullptr) {
            // root item index in base model
            auto it = std::find(base_.begin(), base_.end(), item);
            return std::distance(base_.begin(), it);
        }

        // asking converter to get index for child item
        return conv_.index(item);
    }

    /// The signal is emitted before adding new childs
    mutable signal<void (const value_type &, size_t, size_t)> before_added;

    /// The signal is emitted after adding new childs
    mutable signal<void (const value_type &, size_t, size_t)> after_added;

    /// The signal is emitted before removing childs
    mutable signal<void (const value_type &, size_t, size_t)> before_removed;

    /// The signal is emitted after removing childs
    mutable signal<void (const value_type &, size_t, size_t)> after_removed;

    /// The signal is emitted before changing node
    mutable signal<void (const value_type &)> before_changed;

    /// The signal is emitted after changing node
    mutable signal<void (const value_type &)> after_changed;


private:
    base_model & base_;                         ///< Reference to base vector model object
    Converter conv_;                            ///< Converter instance
    value_type update_item_ = nullptr;          ///< Root item being currently updated

    // connections to base model signals
    scoped_signal_connection before_added_con_;
    scoped_signal_connection after_added_con_;
    scoped_signal_connection before_removed_con_;
    scoped_signal_connection after_removed_con_;
    scoped_signal_connection before_changed_con_;
    scoped_signal_connection after_changed_con_;
};


BOOST_CONCEPT_ASSERT((TreeModel<vector_ptr_tree_model<vector_ptr_model<int>>>));


}


