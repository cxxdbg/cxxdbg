// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file basic_tree_model.hpp
/// Contains definition of the basic_tree_model class.

#pragma once

#include "tree_model.hpp"
#include <ranges.hpp>
#include <initializer_list>
#include <iterator>


namespace cxxdbg {


/// Basic tree model node concept
template <typename Node>
struct BasicTreeModelNode {
public:
    BasicTreeModelNode() = delete;

    BOOST_CONCEPT_USAGE(BasicTreeModelNode) {
        // number of childs
        sz_ = cnode_.childs_size();

        // getting child
        const Node * cn = cnode_.child_at(idx_);
        Node * n = node_.child_at(idx_);

        // getting parent
        cn = cnode_.parent();
        n = node_.parent();

        // getting index of child
        idx_ = cnode_.index(&cnode_);

        // ins
    }

private:
    const Node cnode_;
    Node node_;
    size_t sz_;
    size_t idx_;
};


/// Base class of basic tree model node. Contains common logic for managing child nodes
template <typename Node>
class basic_tree_model_node_base {
public:
    using size_type = size_t;
    using node_up = std::unique_ptr<Node>;

    /// Returns number of child nodes
    size_type childs_size() const { return childs_.size(); }

    /// Returns pointer to const child node at specified index
    const Node * child_at(size_type idx) const {
        assert(idx < childs_size() && "invalid child index");
        return childs_[idx].get();
    }

    /// Returns pointer to child node at specified index
    Node * child_at(size_type idx) {
        assert(idx < childs_size() && "invalid child index");
        return childs_[idx].get();
    }

    /// Returns pointer to const parent node
    const Node * parent() const { return parent_; }

    /// Returns pointer to parent node
    Node * parent() { return parent_; }

    /// Returns index of child node. Node must be child of this node
    size_type index(const Node * child) const {
        auto it = std::find_if(childs_.begin(), childs_.end(), [child](auto && n) {
            return child == n.get();
        });
        assert(it != childs_.end() && "child node not found");
        return std::distance(childs_.begin(), it);
    }

    /// Inserts child nodes at specified index
    template <typename Range>
    void insert(size_type idx, const Range & r) {
        auto sz = std::ranges::size(r);
        childs_.insert(childs_.begin() + idx,
                        std::make_move_iterator(std::ranges::begin(r)),
                        std::make_move_iterator(std::ranges::end(r)));

        for (size_type i = 0; i < sz; ++i) {
            childs_[idx + i]->set_parent(static_cast<Node*>(this));
        }
    }

    /// Inserts single child node
    void insert(size_type idx, std::unique_ptr<Node> && n) {
        std::array<std::unique_ptr<Node>, 1> arr;
        arr[0] = std::move(n);
        insert(idx, arr | std::ranges::views::all);
    }

    /// Remove child nodes from node. Returns vector of unique pointers to removed nodes
    std::vector<node_up> erase(size_type first, size_type last) {
        assert(first <= last && "first index should not be greater than last");
        assert(last < childs_size() && "invalid last index");

        std::vector<node_up> res;
        res.reserve(last - first + 1);

        for (auto idx = first; idx <= last; ++idx) {
            childs_[idx]->set_parent(nullptr);
            res.push_back(std::move(childs_[idx]));
        }

        childs_.erase(childs_.begin() + first, childs_.begin() + last + 1);
        return res;
    }

private:
    /// Sets parent of node
    void set_parent(Node * p) { parent_ = p; }

    Node * parent_ = nullptr;                       ///< Pointer to parent node
    std::vector<std::unique_ptr<Node>> childs_;     ///< Vector of child nodes
};


/// Tree node in basic tree model
template <typename T>
class basic_tree_model_node: public basic_tree_model_node_base<basic_tree_model_node<T>> {
public:
    using size_type = size_t;
    using node_up = std::unique_ptr<basic_tree_model_node>;

    /// Constructs node with specified node value
    basic_tree_model_node(T val): value_{std::move(val)} {}

    /// Returns const reference to value of node
    const T & value() const { return value_; }

    /// Returns reference to value of node
    T & value() { return value_; }

    /// Sets value of node
    void set_value(T val) {
        value_ = std::move(val);
    }

private:
    T value_;                                   ///< Value stored in node
};


BOOST_CONCEPT_ASSERT((BasicTreeModelNode<basic_tree_model_node<int>>));


/// Basic implementation of the TreeModel concept
template <typename Node>
class basic_tree_model {
public:
    /// Type of size
    using size_type = size_t;

    using node = Node;

    /// Type of shared pointer to tree node
    using node_sp = std::shared_ptr<node>;

    /// Type of unique pointer to tree node
    using node_up = std::unique_ptr<node>;

    /// Type alias for value for TreeModel concept that should be pointer to node
    using value_type = const node *;

    /// Type of signal
    template <typename Signature>
    using signal = boost::signals2::signal<Signature>;

    /// Constructs empty tree model
    basic_tree_model() = default;

    /// Destroys tree model
    ~basic_tree_model() = default;

    /// Returns number of childs in specified node
    size_type childs_size(const value_type & parent) const {
        if (parent == nullptr) {
            return root_nodes_.size();
        } else {
            return parent->childs_size();
        }
    }

    /// Returns pointer to child node with specified parent and child index
    value_type child(const value_type & parent, size_t idx) const {
        assert(idx < childs_size(parent) && "invalid child node index");

        if (parent == nullptr) {
            return root_nodes_[idx].get();
        } else {
            return parent->child_at(idx);
        }
    }

    /// Returns range of childs of specified node
    auto childs(const value_type & parent = value_type()) const {
        auto fn = [this, parent](auto idx) { return child(parent, idx); };
        auto indexes = std::ranges::views::iota(size_t(0), childs_size(parent));
        return indexes | std::ranges::views::transform(fn);
    }

    /// Returns pointer parent of node
    value_type parent(const value_type & node) const {
        assert(node != nullptr && "invalid node passed to parent");

        return node->parent();
    }

    /// Returns index of node in parent node
    size_type index(const value_type & node) const {
        if (auto p = node->parent()) {
            return p->index(node);
        }

        auto it = std::find_if(root_nodes_.begin(), root_nodes_.end(), [node](auto && n) {
            return n.get() == node;
        });

        assert(it != root_nodes_.end() && "root node not found");
        return std::distance(root_nodes_.begin(), it);
    }

    /// Adds child nodes to parent at specified index
    template <typename NodesRange>
    void insert_nodes(const value_type & parent, size_type idx, const NodesRange & nodes) {
        assert(idx <= childs_size(parent) && "invalid insert index");

        if (std::size(nodes) == 0) {
            return;
        }

        before_added(parent, idx, idx + std::size(nodes) - 1);

        for (auto && n : nodes) {
            assert(n && "node must not be null");
        }

        if (parent == nullptr) {
            // root node
            root_nodes_.insert(root_nodes_.begin() + idx,
                               std::make_move_iterator(std::ranges::begin(nodes)),
                               std::make_move_iterator(std::ranges::end(nodes)));
        } else {
            const_cast<node*>(parent)->insert(idx, nodes);
        }

        after_added(parent, idx, idx + std::size(nodes) - 1);
    }

    /// Adds child nodes to parent at specified index
    void insert_nodes(const value_type & parent, size_t idx, const std::initializer_list<std::unique_ptr<Node>> & nodes) {
        insert_nodes<>(parent, idx, nodes);
    }

    /// Adds child node to parent at specified index
    void insert_node(const value_type & parent, size_t idx, std::unique_ptr<Node> && node) {
        std::array<std::unique_ptr<Node>, 1> arr;
        arr[0] = std::move(node);
        insert_nodes(parent, idx, arr | std::ranges::views::all);
    }

    /// Removes child nodes from specified parent node. Returns vector of unique pointers to removed nodes
    std::vector<node_up> erase(const value_type & parent, size_type first, size_type last) {
        assert(first <= last && "first index should not be greater than last");
        assert(last < childs_size(parent) && "invalid last node index");

        before_removed(parent, first, last);

        std::vector<node_up> res;

        if (parent == nullptr) {
            for (auto idx = first; idx <= last; ++idx) {
                res.push_back(std::move(root_nodes_[idx]));
            }

            root_nodes_.erase(root_nodes_.begin() + first, root_nodes_.begin() + last + 1);
        } else {
            res = const_cast<node*>(parent)->erase(first, last);
        }

        after_removed(parent, first, last);

        return res;
    }

    mutable signal<void (const value_type &, size_type, size_type)> before_added;
    mutable signal<void (const value_type &, size_type, size_type)> after_added;
    mutable signal<void (const value_type &, size_type, size_type)> before_removed;
    mutable signal<void (const value_type &, size_type, size_type)> after_removed;
    mutable signal<void (const value_type &)> before_changed;
    mutable signal<void (const value_type &)> after_changed;

private:
    std::vector<node_up> root_nodes_;           ///< Vector of root nodes
};


BOOST_CONCEPT_ASSERT((TreeModel<basic_tree_model<basic_tree_model_node<int>>>));


}
