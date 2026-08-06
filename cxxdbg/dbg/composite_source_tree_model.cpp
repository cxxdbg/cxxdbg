// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "composite_source_tree_model.hpp"
#include "cxxdbg/app/tree_view_model.hpp"
#include "source_tree.hpp"


namespace fs = std::filesystem;


namespace cxxdbg::dbg {


/// Returns pair of first and last source tree items for composite node item
static auto get_composite_first_last(const source_tree & tree, const source_tree_item * item) {
    if (!item->is_directory()) {
        return std::pair(item, item);
    }

    auto first = item;
    while (true) {
        auto p = tree.parent(first);
        if (p == nullptr || tree.childs_size(p) != 1) {
            break;
        }

        assert(p->is_directory() && "parent node must be a directory");

        first = p;
    }

    auto last = item;
    while (true) {
        if (tree.childs_size(last) != 1) {
            break;
        }

        auto ch = tree.child(last, 0);
        if (!ch->is_directory()) {
            break;
        }

        last = ch;
    }

    return std::pair{first, last};
}


composite_source_tree_model::composite_source_tree_model(source_tree & src):
sources_{src} {
    src.before_added.connect([this](auto && parent, auto first_idx, auto last_idx) {
        if (parent != nullptr) {
            assert(parent->is_directory() && "parent must be a directory");
            auto [first, last] = get_composite_first_last(sources_, parent);

            if (// inserting in the middle of directory chain
                parent != last ||

                // adding single item into empty directory (or directory chain)
                // It's possible that directory will be inserted and we need extend parent chain.
                (sources_.childs_size(parent) == 0 && first_idx == 0 && last_idx == 0))
            {

                // We need emulate removed node which is start of the chain

                auto chain_idx = sources_.index(first);
                auto chain_parent = sources_.parent(first);
                before_removed()(make_index(chain_parent), chain_idx, chain_idx);

                removed_node_index_ = chain_idx;
                removed_node_parent_ = chain_parent;
                removed_node_ = first;
                save_removed_expanded_childs(first);

                after_removed()(make_index(chain_parent), chain_idx, chain_idx);

                return;
            }
        }

        before_added()(make_index(parent), first_idx, last_idx);
    });

    src.after_added.connect([this](auto && parent, auto first_idx, auto last_idx) {
        if (parent != nullptr) {
            if (removed_node_index_ != SIZE_MAX) {
                // we are emulating removed node and need stop it here
                
                auto removed_idx = removed_node_index_;
                auto removed_parent = removed_node_parent_;
                assert(removed_idx != SIZE_MAX && "invalid removed node index");
                
                before_added()(make_index(removed_parent), removed_idx, removed_idx);
                removed_node_index_ = SIZE_MAX;
                removed_node_parent_ = nullptr;
                removed_node_ = nullptr;
                after_added()(make_index(removed_parent), removed_idx, removed_idx);

                // expanding all previously expanded nodes
                for (auto && n : removed_node_expanded_childs_) {
                    auto [first, last] = get_composite_first_last(sources_, n);
                    // expanding only last nodes in composite chain because all other
                    // nodes in chain are not visible to client
                    if (n == last) {
                        expand_row()(make_index(n));
                    }
                }

                removed_node_expanded_childs_.clear();

                return;
            }
        }

        after_added()(make_index(parent), first_idx, last_idx);
    });

    src.before_removed.connect([this](auto && ... args) {
        assert(false && "before_removed should never be emitted");
    });

    src.after_removed.connect([this](auto && ... args) {
        assert(false && "after_removed should never be emitted");
    });

    src.before_changed.connect([this](auto && ... args) {
        assert(false && "before_changed should never be emitted");
    });

    src.after_changed.connect([this](auto && ... args) {
        assert(false && "after_changed should never be emitted");
    });
}


size_t composite_source_tree_model::childs_size(const row_index & row) const {
    auto node = tree_node(row);
    auto sz = sources_.childs_size(tree_node(row));

    if (removed_node_index_ != SIZE_MAX && node == removed_node_parent_) {
        return sz - 1;
    }

    return sz;
}


auto composite_source_tree_model::child(const row_index & row, std::size_t index) const -> row_index {
    auto node = tree_node(row);
    auto real_index = index;

    if (removed_node_index_ != SIZE_MAX) {
        if (removed_node_parent_ == node && removed_node_index_ <= index) {
            ++real_index;
        }
    }

    auto ch = sources_.child(node, real_index);
    auto [first, last] = get_composite_first_last(sources_, ch);
    assert(ch == first && "accessed child node must be first in node chain");
    return make_index(last);
}


size_t composite_source_tree_model::index(const row_index & row) const {
    assert(row.is_valid() && "invalid row index");
    auto node = tree_node(row);
    auto [first, last] = get_composite_first_last(sources_, node);
    assert(node == last && "node accessed from composite model must be the last in chain");

    auto idx = sources_.index(first);

    if (removed_node_index_ != SIZE_MAX) {
        if (sources_.parent(first) == removed_node_parent_ && removed_node_index_ < idx) {
            assert(idx > 0 && "invalid logic in removed node emulation");
            --idx;
        }
    }

    return idx;
}


auto composite_source_tree_model::parent(const row_index & row) const -> row_index {
    assert(row.is_valid() && "invalid row index");
    auto node = tree_node(row);
    auto [first, last] = get_composite_first_last(sources_, node);
    assert(node == last && "node accessed from composite model must be the last in chain");
    return make_index(sources_.parent(first));
}


std::wstring composite_source_tree_model::text(const row_index & row, std::size_t c) const {
    assert(c == 0 && "invalid column index");
    auto node = tree_node(row);
    auto [first, last] = get_composite_first_last(sources_, node);
    assert(node == last && "node accessed from composite model must be the last in chain");

    fs::path res = first->name();
    auto cnode = first;
    while (cnode != last) {
        assert(sources_.childs_size(cnode) == 1 && "invalid node chain");
        cnode = sources_.child(cnode, 0);

        res /= cnode->name();
    }

    res.make_preferred();
    return res.wstring();
}


auto composite_source_tree_model::image(const row_index & row, std::size_t c) const -> image_index {
    assert(c == 0 && "invalid column index");
    if (tree_node(row)->is_directory()) {
        return image_index_directory;
    }

    return image_index_file;
}


auto composite_source_tree_model::get_expand_state(const row_index & index) const -> tree_view_expand_state {
    return tree_view_expand_state::undefined;
}


void composite_source_tree_model::set_expand_state(const row_index & index, tree_view_expand_state state) const {
    auto node = tree_node(index);
    assert(node && "invalid tree node");

    auto [first, last] = get_composite_first_last(sources_, node);
    while (true) {
        if (state == tree_view_expand_state::expanded) {
            expanded_nodes_.insert(first);
        } else {
            expanded_nodes_.erase(first);
        }

        if (first == last) {
            break;
        }

        assert(sources_.childs_size(first) == 1 && "invalid composite node chain");
        first = sources_.child(first, 0);
    }
}


auto composite_source_tree_model::path_index(const fs::path & p) const -> row_index {
    assert(!p.empty() && "invalid path");
    auto item = sources_.path_item(p);
    return make_index(item);
}


const source_tree_item * composite_source_tree_model::tree_node(const row_index & idx) const {
    return reinterpret_cast<const source_tree_item*>(idx.ptr());
}


auto composite_source_tree_model::make_index(const source_tree_item * item) const -> row_index {
    return ro_tree_view_model::make_index(const_cast<source_tree_item*>(item));
}


void composite_source_tree_model::save_removed_expanded_childs(const source_tree_item * node) {
    if (expanded_nodes_.count(node) > 0) {
        removed_node_expanded_childs_.insert(node);
    }

    for (auto && n : sources_.childs(node)) {
        save_removed_expanded_childs(n);
    }
}


}
