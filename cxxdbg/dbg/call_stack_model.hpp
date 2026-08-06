// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file call_stack_model.hpp
/// Contains definition of the call_stack_model class.

#pragma once

#include <deque>
#include <regex>

#include "call_stack_tree.hpp"
#include "stack_frame.hpp"
#include "cxxdbg/app/tree_view_model.hpp"


namespace cxxdbg::dbg {


class stack_frame;
class thread;


/// Tree view model for thread call stack
class call_stack_model: public ro_tree_view_model {
public:
    /// Image index
    enum call_stack_image: image_index {
        call_stack_image_none = 0,
        call_stack_image_null_frame,
        call_stack_image_current_frame
    };

    /// Constructor, makes call stack model with specified reference
    /// to thread object
    explicit call_stack_model(const thread * thrd = nullptr, const std::string & group_functions_regex = {});

    /// Destructor, destoys object
    ~call_stack_model() override;

    /// Sets thread for call stack model
    void set_thread(const thread * thrd = nullptr);

    /// Returns true if parameter types should be displayed
    bool disp_par_types() const;

    /// Sets if parameter types should be displayed
    void set_disp_par_types(bool v);

    /// Returns true if parameter names should be displayed
    bool disp_par_names() const;

    /// Sets if parameter names should be displayed
    void set_disp_par_names(bool v);

    /// Returns true if parameter values should be displayed
    bool disp_par_vals() const;

    /// Sets if parameter values should be displayed
    void set_disp_par_vals(bool v);

    /// Returns number of columns
    std::size_t columns_size() const override;

    /// Returns name of column with specified index
    std::wstring column_name(std::size_t index) const override;

    /// Returns number of child rows for row
    std::size_t childs_size(const row_index & row) const override;

    /// Returns child row with specified index
    row_index child(const row_index & row, std::size_t index) const override;

    /// Returns index of row in parent
    std::size_t index(const row_index & row) const override;

    /// Returns parent for specified row
    row_index parent(const row_index & row) const override;

    /// Returns text for specified cell
    std::wstring text(const row_index & row, std::size_t c) const override;

    /// Returns image index for specified cell
    image_index image(const row_index & row, std::size_t c) const override;

    /// Returns pointer to stack frame with corresponding to specified row,
    /// or null for row with ... for incomplete call stack
    const stack_frame * frame(const row_index & row) const {
        auto frm = make_tree_node(row);
        if (frm == &fake_node_) {
            return nullptr;
        }

        return frm;
    }

    /// group functions lists signal handler
    void group_functions_lists_changed(const std::string & value);

private:
    /// Makes stack frame pointer from row index
    static const stack_frame * make_tree_node(const row_index & row);

    /// Makes tree view model index from pointer to stack frame
    static row_index make_index(const stack_frame * frame);

    /// Sends after changed signal for all stack frames with parameters
    void update_params();

    /// Removes virtual fake node
    void remove_fake_node();

    /// Adds virtual fake node
    void add_fake_node();

    scoped_signal_connection on_before_complete_call_stack_changed_;    ///< Connection holder
    scoped_signal_connection on_after_complete_call_stack_changed_;     ///< Connection holder

    const thread * thrd_;       ///< Pointer to thread object
    bool disp_par_types_;       ///< Should display parameter types?
    bool disp_par_names_;       ///< Should display parameter names?
    bool disp_par_vals_;        ///< Should display parameter values?

    bool has_fake_node_;        ///< Indicates that stack is incomplete
    call_stack_tree tree_;      ///< Call stack tree implementation

    /// Fake frame for '...' node
    stack_frame fake_node_{{0, 0, {}}, 0, 0};

};

}


