// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file tree_model.hpp
/// Contains definition of the TreeModel concept

#pragma once

#include "signals.hpp"
#include <boost/concept_check.hpp>


namespace cxxdbg {


/// Concept of tree model
template <typename T>
class TreeModel {
    using value_type = typename T::value_type;
    using size_type = typename T::size_type;

public:
    TreeModel() = delete;

    BOOST_CONCEPT_USAGE(TreeModel) {
        // number of childs
        sz_ = mdl_.childs_size(node_);

        // getting child
        value_type n = mdl_.child(node_, idx_);

        // getting parent
        n = mdl_.parent(node_);

        // getting index of child
        idx_ = mdl_.index(node_);

        // signals
        con_ = mdl_.before_added.connect([](const value_type &, size_type, size_type) {});
        con_ = mdl_.after_added.connect([](const value_type &, size_type, size_type) {});
        con_ = mdl_.before_removed.connect([](const value_type &, size_type, size_type) {});
        con_ = mdl_.after_removed.connect([](const value_type &, size_type, size_type) {});
        con_ = mdl_.before_changed.connect([](const value_type &) {});
        con_ = mdl_.after_changed.connect([](const value_type &) {});
    }

private:
    const T mdl_;
    const value_type node_;
    size_t sz_;
    size_t idx_;
    signal_connection con_;
};


/// Base class for tree models that contains required typedefs and signal definitions
template <typename T, typename SignalT>
class tree_model_base {
public:
    using value_type = T;
    using size_type = size_t;

    tree_model_base() = default;

    // noncopyable
    tree_model_base(const tree_model_base &) = delete;
    tree_model_base & operator=(const tree_model_base &) = delete;

    mutable boost::signals2::signal<void (SignalT, size_type, size_type)> before_added;
    mutable signal<void (SignalT, size_type, size_type)> after_added;
    mutable signal<void (SignalT, size_type, size_type)> before_removed;
    mutable signal<void (SignalT, size_type, size_type)> after_removed;
    mutable signal<void (SignalT)> before_changed;
    mutable signal<void (SignalT)> after_changed;
};


}


