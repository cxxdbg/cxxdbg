// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file flat_tree_model.hpp
/// Contains definition of the flat_tree_model class.

#pragma once

#include "basic_tree_model.hpp"
#include <ranges.hpp>
#include <memory>


namespace cxxdbg {


/// Tree model that implements TreeModel concept for specified node type and stores
/// actual tree in basic_tree_model with additional map from nodes to basic_tree_model nodes
template <typename T>
class flat_tree_model: virtual public tree_model_base<T, T> {
public:
    using value_type = T;
    using size_type = size_t;

    /// Constructs empty model
    flat_tree_model() {
        impl_.before_added.connect([this](auto node, auto first, auto last) {
            this->before_added(map_from_node(node), first, last);
        });

        impl_.after_added.connect([this](auto node, auto first, auto last) {
            // adding new nodes into map
            for (auto i = first; i <= last; ++i) {
                auto n = impl_.child(node, i);
                auto val = map_from_node(n);

                auto res = node_map_.emplace(val, n);
                assert(res.second && "node already exists");
            }

            this->after_added(map_from_node(node), first, last);
        });

        impl_.before_removed.connect([this](auto node, auto first, auto last) {
            this->before_removed(map_from_node(node), first, last);

            // removing nodes from map
            for (auto i = first; i <= last; ++i) {
                auto n = impl_.child(node, i);
                remove_nodes_from_map(n);
            }
        });

        impl_.after_removed.connect([this](auto node, auto first, auto last) {
            this->after_removed(map_from_node(node), first, last);
        });

        impl_.before_changed.connect([this](auto node) {
            this->before_changed(map_from_node(node));
        });

        impl_.after_changed.connect([this](auto node) {
            this->after_changed(map_from_node(node));
        });
    }

    /// Destroys model
    ~flat_tree_model() = default;

    /// Returns number of childs of node
    size_type childs_size(const T & val) const {
        return impl_.childs_size(map_to_node(val));
    }

    /// Returns child of node
    T child(const T & parent, size_type idx) const {
        assert(idx < childs_size(parent) && "invalid index of child node");
        return map_from_node(impl_.child(map_to_node(parent), idx));
    }

    /// Returns range of childs of node
    auto childs(const T & parent) const {
        auto fn = [this](auto && node) { return map_from_node(node); };
        return impl_.childs(map_to_node(parent)) | std::ranges::views::transform(fn);
    }

    /// Returns parent node
    T parent(const T & val) const {
        auto p_node = impl_.parent(map_to_node(val));
        if (p_node == nullptr) {
            return T();
        }

        return map_from_node(p_node);
    }

    /// Returns index of node in parent node
    size_type index(const T & val) const {
        return impl_.index(map_to_node(val));
    }

    /// Inserts child nodes into node
    template <typename Range>
    void insert(const T & parent, size_type idx, const Range & childs) {
        auto fn = [](auto && val) { return std::make_unique<basic_tree_model_node<T>>(std::move(val)); };
        impl_.insert_nodes(map_to_node(parent), idx, childs | std::ranges::views::transform(fn));
    }

    /// Inserts child nodes into node
    void insert(const T & parent, size_type idx, const std::initializer_list<T> & childs) {
        insert<>(parent, idx, childs);
    }

    /// Removes child nodes from parent node
    void erase(const T & parent, size_type first, size_type last) {
        impl_.erase(map_to_node(parent), first, last);
    }

    /// Removes all content of tree
    void clear() {
        erase(T(), 0, childs_size(T()) - 1);
    }

private:
    /// Type of underlying model
    using impl_t = basic_tree_model<basic_tree_model_node<T>>;

    /// Type of node in underlying model
    using node = typename impl_t::node;

    /// Maps value to node from implementation model
    const node * map_to_node(const T & val) const {
        if (val == T()) {
            return nullptr;
        }

        auto it = node_map_.find(val);
        assert(it != node_map_.end() && "node not found");
        return it->second;
    }

    /// Maps node to value
    T map_from_node(const node * n) const {
        if (n == nullptr) {
            return T();
        }

        return n->value();
    }

    /// Recursively removes node and all its child nodes from map
    void remove_nodes_from_map(const node * n) {
        // removing node
        auto cnt = node_map_.erase(n->value());
        assert(cnt > 0 && "node not found");

        // removing child nodes
        for (auto && c : impl_.childs(n)) {
            remove_nodes_from_map(c);
        }
    }


    impl_t impl_;                          ///< Implementation model
    std::map<T, const node*> node_map_;    ///< Map from values to tree nodes
};


}
