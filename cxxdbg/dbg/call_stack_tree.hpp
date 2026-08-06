// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

//
// Created by yzarudny on 18.11.17.
//

#pragma once

#include "thread.hpp"
#include "cxxdbg/app/flat_tree_model.hpp"
#include "cxxdbg/app/signals.hpp"
#include "cxxdbg/app/tree_view_model.hpp"
#include "cxxdbg/dbg/core/regex_lists.hpp"

#include <cassert>
#include <regex>
#include <regex>
#include <unordered_map>


namespace cxxdbg::dbg {


/// Call stack tree model implementation
class call_stack_tree: private flat_tree_model<const stack_frame*>,
                       virtual public tree_model_base<const stack_frame*, const stack_frame*> {

    using base_type = flat_tree_model<const stack_frame*>;

public:
    /// Constructs tree with specified grouping regex
    call_stack_tree(std::regex group_regex);

    /// Returns number of child nodes in parent node
    size_type childs_size(const stack_frame * parent = nullptr) const {
        return base_type::childs_size(parent);
    }

    /// Returns child frame with specified index in parent
    const stack_frame * child(const stack_frame * parent, size_t idx) const {
        return base_type::child(parent, idx);
    }

    auto childs(const stack_frame * parent) const {
        return base_type::childs(parent);
    }

    /// Returns parent of frame
    const stack_frame * parent(const stack_frame * frame) const {
        return base_type::parent(frame);
    }

    /// Returns index of stack frame in parent node
    size_type index(const stack_frame * frame) const {
        return base_type::index(frame);
    }

    /// Calculates real frame index in call stack
    size_t calculate_frame_index(const stack_frame * frame) const;

    /// Sets current thread and rebuilds tree model
    void set_thread(const thread * thrd);

    /// Sets grouping regex
    void set_group_regex(std::regex r);

private:
    /// Connects to thread signals
    void connect();

    /// Disconnects from thread signals
    void disconnect();

    /// Rebuilds tree model
    void rebuild();

    /// Removes all content from model
    void clear();

    /// Matches frame with grouping regex
    bool match(const stack_frame * frame) const;

    /// Inserts frames into tree at specified position in root.
    void add_frames(size_type first, size_type last, size_type insert_idx);

    std::regex group_regex_;        ///< Grouping regex
    const thread * thrd_{nullptr};  ///< Active thread

    // signal connections
    scoped_signal_connection on_before_removed_connection_;
    scoped_signal_connection on_after_added_connection_;
    scoped_signal_connection on_after_changed_connection_;
};


}


