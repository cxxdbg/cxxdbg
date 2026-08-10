// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

//
// Created by yzarudny on 18.11.17.
//

#include <iostream>

#include "call_stack_tree.hpp"
#include "stack_frame.hpp"
#include "cxxdbg/util/convert.hpp"
#include <ranges.hpp>
#include <sstream>


#define DEBUG_CALL_STACK_TREE

namespace cxxdbg::dbg {


call_stack_tree::call_stack_tree(std::regex group_regex):
group_regex_{std::move(group_regex)} {
}


size_t call_stack_tree::calculate_frame_index(const stack_frame * frame) const {
    const size_t pi = index(frame);

    size_t result = pi + 1;
    auto p = parent(frame);
    if (p) {
        const size_t ppi = index(p);
        result += ppi + 1;
        for (size_t i = 0; i < ppi; ++i) {
            auto f = child(nullptr, i);
            result += childs_size(f);
        }
    } else {
        for (size_t i = 0; i < pi; ++i) {
            auto f = child(nullptr, i);
            result += childs_size(f);
        }
    }

    return result;
}


void call_stack_tree::set_thread(const thread * thrd) {
    clear();
    disconnect();
    thrd_ = thrd;
    connect();
    rebuild();
}


void call_stack_tree::set_group_regex(std::regex r) {
    clear();
    group_regex_ = std::move(r);
    rebuild();
}



void call_stack_tree::connect() {
    if (!thrd_)
        return;

    assert(childs_size(nullptr) == 0 && "tree is not empty when connecting to new thread");

    on_before_removed_connection_ = thrd_->before_stack_frames_removed.connect([&](size_t f, size_t l) {
        const size_t stack_size = thrd_->call_stack().size();
        assert((f == 0 || l == stack_size - 1) && "remove from the beginning or from the end only");

        assert(f < stack_size && "index out of bound");
        assert(l < stack_size && "index out of bound");

        auto * f_frame = thrd_->call_stack()[f];
        auto * l_frame = thrd_->call_stack()[l];

        auto f_parent = parent(f_frame);
        auto l_parent = parent(l_frame);

        // // indexes of root nodes that should be completely removed with all their childs
        size_t first_root_node_to_remove_idx = SIZE_MAX;
        size_t last_root_node_to_remove_idx = SIZE_MAX;

        // vector of child nodes that should be added to parent of the first node or into
        // new root node created from the first element
        std::vector<const stack_frame*> new_childs;

        if (f_parent == nullptr) {
            // first node is root node

            first_root_node_to_remove_idx = index(f_frame);

            if (l_parent == f_frame) {
                // the first node is the parent of the last node

                last_root_node_to_remove_idx = first_root_node_to_remove_idx;
                auto l_node_idx = index(l_frame);

                if (l_node_idx == childs_size(f_frame) - 1) {
                    // we need only completely remove first node and all its childs
                } else {
                    // we need remove all [0, index(l_node)] childs from the first node, and first node itself.
                    // After that, the child node with index(l_node) + 1 index becomes new root node, and
                    // all other child nodes become its childs.

                    assert(l_node_idx + 1 < childs_size(f_frame) && "invalid last node index");
                    auto app_childs = childs(f_frame) | std::ranges::views::drop(l_node_idx);
                    for (auto && ch : app_childs) {
                        new_childs.push_back(ch);
                    }
                }
            } else {
                // we need completely remove the first node
            }

        } else {
            auto f_node_idx = index(f_frame);
            if (l_parent == f_parent) {
                // last node is in same parent as first node.
                // We need just remove child nodes from same parent node and return
                erase(f_parent, f_node_idx, index(l_frame));
                return;
            }

            // removing all childs after first node from parent node of the first node
            assert(childs_size(f_parent) > 0 && "parent must not be empty");
            erase(f_parent, f_node_idx, childs_size(f_parent) - 1);

            // fist root node to remove is the node next after the parent of the first node
            first_root_node_to_remove_idx = index(f_parent) + 1;
            assert(first_root_node_to_remove_idx < childs_size() && "invalid first parent index");
        }

        // calculating last root node to remove if it's not calculated yet
        if (last_root_node_to_remove_idx == SIZE_MAX) {
            if (l_parent == nullptr) {
                // last node is root node

                last_root_node_to_remove_idx = index(l_frame);
                // we need add all childs of the last node to the parent of the first node  or newly created root node
                auto app_childs = childs(l_frame);
                for (auto && ch : app_childs) {
                    new_childs.push_back(ch);
                }

            } else {
                last_root_node_to_remove_idx = index(l_parent);

                auto l_node_idx = index(l_frame);

                // we need add all childs after the last node to the parent of the first node or newly created root node
                auto app_childs = childs(l_parent) | std::ranges::views::drop(l_node_idx);
                for (auto && ch : app_childs) {
                    new_childs.push_back(ch);
                }
            }
        }

        // removing root nodes
        assert(first_root_node_to_remove_idx != SIZE_MAX && "first root node index is invalid");
        assert(last_root_node_to_remove_idx != SIZE_MAX && "last root node index is invalid");
        erase(nullptr, first_root_node_to_remove_idx, last_root_node_to_remove_idx);

        // adding new child nodes to parent of the first node or newly created node
        if (!new_childs.empty()) {
            if (f_parent == nullptr) {
                // using the first child as root node
                auto new_root = new_childs.front();
                insert(nullptr, first_root_node_to_remove_idx, {new_root});
                insert(new_root, 0, new_childs | std::ranges::views::drop(1));
            } else {
                // adding childs to the parent of the first node
                insert(f_parent, childs_size(f_parent), new_childs);
            }
        }
    });

    on_after_added_connection_ = thrd_->after_stack_frames_added.connect([&](size_t f, size_t l) {
        const size_t stack_size = thrd_->call_stack().size();

        size_type insert_idx = 0;
        if (f != 0) {
            auto prev_frame = thrd_->call_stack()[f - 1];
            auto prev_frame_parent = parent(prev_frame);
            if (prev_frame_parent != nullptr) {
                insert_idx = index(prev_frame_parent) + 1;
            } else {
                insert_idx = index(prev_frame) + 1;
            }
        }

        add_frames(f, l, insert_idx);
    });

    on_after_changed_connection_ = thrd_->stack_frame_changed.connect([&](std::size_t idx) {
        auto * frame = thrd_->call_stack()[idx];
        assert(frame != nullptr && "invalid frame in stack");
        after_changed(frame);
    });
}


void call_stack_tree::disconnect() {
    on_after_added_connection_.disconnect();
    on_before_removed_connection_.disconnect();
    on_after_changed_connection_.disconnect();
}


void call_stack_tree::rebuild() {
    if (!thrd_) {
        return;
    }

    if (thrd_->call_stack().size() != 0) {
        add_frames(0, thrd_->call_stack().size() - 1, 0);
    }
}


void call_stack_tree::clear() {
    if (childs_size() == 0) {
        return;
    }

    erase(nullptr, 0, childs_size() - 1);
}


bool call_stack_tree::match(const stack_frame * frame) const {
    std::wostringstream stream;
    frame->print(stream, false, false, false);
    return std::regex_search(util::convert::to_string(stream.str()), group_regex_);
}


void call_stack_tree::add_frames(size_type first, size_type last, size_type insert_idx) {
    const stack_frame * last_parent = nullptr;

    // if old last root node matches grouping criteria then adding all matched new nodes into it
    if (insert_idx != 0) {
        assert(insert_idx <= childs_size(nullptr) && "invalid insert index");
        last_parent = child(nullptr, insert_idx - 1);
    }

    for (size_t i = first; i <= last; ++i) {
        const stack_frame * frame = thrd_->call_stack()[i];
        if (match(frame)) {
            if (last_parent == nullptr) {
                // new root node
                last_parent = frame;
                insert(nullptr, insert_idx, {frame});
                ++insert_idx;
            } else {
                // adding child to last parent
                insert(last_parent, childs_size(last_parent), {frame});
            }
        } else {
            last_parent = frame;
            insert(nullptr, insert_idx, {frame});
            ++insert_idx;
        }
    }

    assert(last_parent != nullptr && "invalid last parent node");

    // if first node after inserted nodes matches grouping cirteria then add it and all its
    // children into last inserted parent node
    if (insert_idx < childs_size(nullptr)) {
        auto next_frame = child(nullptr, insert_idx);
        if (match(next_frame)) {
            auto chlds = childs(next_frame);
            // NOTE: for-like loop is required here for workaround the bug in clang-12 with mingw/libstdc++ on
            // on Windows. Invoking vector constructor with iterators returned from begin/end for the
            // chlds range fails to compile.
            std::vector<const stack_frame*> next_childs;
            for (auto && c : chlds) {
                next_childs.push_back(c);
            }

            erase(nullptr, insert_idx , insert_idx);
            insert(last_parent, childs_size(last_parent), {next_frame});
            insert(last_parent, childs_size(last_parent), next_childs);
        }
    }
}


}
