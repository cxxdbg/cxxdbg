// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file call_stack_model.cpp
/// Contains implementation of the call_stack_model class.

#include "call_stack_model.hpp"
#include "stack_frame.hpp"
#include "thread.hpp"

#include <cassert>
#include <sstream>
#include <algorithm>
#include <regex>
#include <iostream>


namespace cxxdbg::dbg {


call_stack_model::call_stack_model(const thread * thrd, const std::string & group_functions_regex):
tree_{group_functions_regex.empty() ? std::regex{} : std::regex(group_functions_regex)},
thrd_{nullptr},
disp_par_types_{false},
disp_par_names_{false},
disp_par_vals_{false},
has_fake_node_{false} {

    set_thread(thrd);

    // no need to remember scoped connections, tree will be destroyed before model
    // with all the connections it created

    tree_.before_added.connect([this](const stack_frame * parent, size_t first, size_t last) {
        before_added()(make_index(parent), first, last);
    });

    tree_.after_added.connect([this](const stack_frame * parent, size_t first, size_t last) {
        after_added()(make_index(parent), first, last);
    });

    tree_.before_removed.connect([this](const stack_frame * parent, size_t first, size_t last) {
        before_removed()(make_index(parent), first, last);
    });

    tree_.after_removed.connect([this](const stack_frame * parent, size_t first, size_t last) {
        after_removed()(make_index(parent), first, last);
    });

    tree_.after_changed.connect([this](const stack_frame * node) {
        after_changed()(make_index(node));
    });
}


call_stack_model::~call_stack_model() {
}


void call_stack_model::set_thread(const thread * thrd) {
#if false
    std::cout << "set thread " << thrd << " old thread = " << thrd_ << std::endl;
#endif
    // add connections to incomplete_flag_changed

    if (thrd == thrd_)
        return;

    const thread * old_thrd = thrd_;

    if (old_thrd) {
        on_before_complete_call_stack_changed_.disconnect();
        on_after_complete_call_stack_changed_.disconnect();

        // process incomplete stack status
        remove_fake_node(); // if it exists
        thrd_ = nullptr;
        tree_.set_thread(nullptr);
    }

    const thread * new_thrd = thrd;

    if (new_thrd) {
        thrd_ = new_thrd;

        if (!thrd_->has_complete_call_stack()) {
            add_fake_node();
        } else {
            remove_fake_node();
        }

        on_before_complete_call_stack_changed_ = thrd_->before_complete_call_stack_changed.connect([&]() {
            size_t index = tree_.childs_size();

            if (!thrd_->has_complete_call_stack()) {
                before_removed()({}, index, index);
            } else {
                before_added()({}, index, index);
            }
        });

        on_after_complete_call_stack_changed_ = thrd_->after_complete_call_stack_changed.connect([&]() {
            size_t index = tree_.childs_size();

            if (thrd_->has_complete_call_stack()) {
                has_fake_node_ = false;
                after_removed()({}, index, index);
            } else {
                has_fake_node_ = true;
                after_added()({}, index, index);
            }
        });

        tree_.set_thread(thrd_);
    }
}


bool call_stack_model::disp_par_types() const {
    return disp_par_types_;
}


void call_stack_model::set_disp_par_types(bool v) {
    if (disp_par_types_ != v) {
        disp_par_types_ = v;
        update_params();
    }
}


bool call_stack_model::disp_par_names() const {
    return disp_par_names_;
}


void call_stack_model::set_disp_par_names(bool v) {
    if (disp_par_names_ != v) {
        disp_par_names_ = v;
        update_params();
    }
}


bool call_stack_model::disp_par_vals() const {
    return disp_par_vals_;
}


void call_stack_model::set_disp_par_vals(bool v) {
    if (disp_par_vals_ != v) {
        disp_par_vals_ = v;
        update_params();
    }
}


std::size_t call_stack_model::columns_size() const {
    return 3;
}


std::wstring call_stack_model::column_name(std::size_t index) const {
    switch (index) {
    case 0:
        return {};
    case 1:
        return L"N";
    case 2:
        return L"Location";
    default:
        assert(false && "invalid column index");
    }

    return {};
}


std::size_t call_stack_model::childs_size(const row_index & row) const {
    if (!row.is_valid() && has_fake_node_) {
        return tree_.childs_size() + 1;
    }

    auto frame = make_tree_node(row);
    if (frame == &fake_node_) {
        return 0;
    }

    return tree_.childs_size(frame);
}


call_stack_model::row_index call_stack_model::child(const row_index & row, std::size_t index) const {
    assert(index < childs_size(row) && "invalid child index");
    if (!row.is_valid() && has_fake_node_ && index == tree_.childs_size()) {
        return make_index(&fake_node_);
    }

    return make_index(tree_.child(make_tree_node(row), index));
}


std::size_t call_stack_model::index(const row_index & row) const {
    auto frame = make_tree_node(row);
    if (frame == &fake_node_) {
        return tree_.childs_size();
    }

    return tree_.index(frame);
}


call_stack_model::row_index call_stack_model::parent(const row_index & row) const {
    auto frame = make_tree_node(row);
    if (frame == &fake_node_) {
        return make_index(nullptr);
    }

    return make_index(tree_.parent(frame));
}


std::wstring call_stack_model::text(const row_index & row, std::size_t c) const {
    if (c == 0) {
        // text for the first column is always empty
        return {};
    }

    auto frame = make_tree_node(row);
    if (frame == &fake_node_) {
        // last fake node
        return L"...";
    }

    switch (c) {
    case 1: {
        auto frame_index = tree_.calculate_frame_index(frame);
        return std::to_wstring(frame_index);

    }
    case 2: {
        std::wostringstream str;
        frame->print(str, disp_par_types(), disp_par_names(), disp_par_vals());
        return str.str();
    }
    default:
        assert(false && "invalid column index");
    }

    return {};
}


call_stack_model::image_index
call_stack_model::image(const row_index & row, std::size_t c) const {
    if (c != 0) {
        return call_stack_image_none;
    }

    auto frame = make_tree_node(row);
    if (frame == &fake_node_) {
        return call_stack_image_none;
    }

    // check if top frame
    bool is_null_frame = frame == tree_.child(nullptr, 0);
    if (is_null_frame) {
        return call_stack_image_null_frame;
    }

    size_t current_index = thrd_->current_frame_index();
    if (current_index == thread::invalid_current_frame) {
        return call_stack_image_none;
    }

    assert(current_index < thrd_->call_stack().size() && "current frame index out of bound");

    auto current_frame = thrd_->call_stack()[current_index];
    if (current_frame == frame) {
        return call_stack_image_current_frame;
    }

    return call_stack_image_none;
}


// updates data in view for all items
void call_stack_model::update_params() {
    if (!thrd_)
        return;

    for (auto && c : tree_.childs(nullptr)) {
        after_changed()(make_index(c));
        for (auto && cc : tree_.childs(c)) {
            after_changed()(make_index(cc));
        }
    }
}


void call_stack_model::group_functions_lists_changed(const std::string & value) {
    tree_.set_group_regex(value.empty() ? std::regex{} : std::regex{value});
}

const stack_frame * call_stack_model::make_tree_node(const ro_tree_view_model::row_index & row) {
    return reinterpret_cast<stack_frame *>(row.ptr());
}


call_stack_model::row_index call_stack_model::make_index(const stack_frame * frame) {
    return ro_tree_view_model::make_index(const_cast<stack_frame*>(frame));
}


void call_stack_model::remove_fake_node() {
    if (has_fake_node_) {
        size_t index = tree_.childs_size(nullptr);
        before_removed()({}, index, index);
        has_fake_node_ = false;
        after_removed()({}, index, index);
    }
}

void call_stack_model::add_fake_node() {
    if (!has_fake_node_) {
        size_t index = tree_.childs_size(nullptr);
        before_added()({}, index, index);
        has_fake_node_ = true;
        after_added()({}, index, index);
    }
}

}
