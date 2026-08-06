// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "cxxdbg/app/tree_view_model.hpp"

#include <deque>
#include <string>


namespace cxxdbg::dbg {

namespace cli {
    class app_command_interpreter;
}


/// class auto_completion_data incapsulates auto completion model data
class auto_completion_data {
public:
    /// constructor
    auto_completion_data(cli::app_command_interpreter & interp, ro_tree_view_model & model);

    /// handles prefix changed event
    void handle_prefix_changed(const std::string & prefix);

    /// returns number of items
    std::size_t size() const { return  commands_.size(); }

    /// returns item at index
    const std::wstring & child_at(std::size_t i) const { return commands_[i]; }

private:
    typedef ro_tree_view_model::childs_signal_func childs_signal_func;
    /// Before added signal, emitted before adding new row
    CXXDBG_DEFINE_SIGNALX(before_added, childs_signal_func)

    /// After added signal, emitted after adding new row
    CXXDBG_DEFINE_SIGNALX(after_added, childs_signal_func)

    /// Before removed signal, emitted before removing row
    CXXDBG_DEFINE_SIGNALX(before_removed, childs_signal_func)

    /// After removed signal, emitted after removig row
    CXXDBG_DEFINE_SIGNALX(after_removed, childs_signal_func)

    cli::app_command_interpreter & interp_;     /// Reference to interpreter
    std::deque<std::wstring> commands_;         /// Commands container
};


/// class auto_complemetion__model is model implementation for auto completion widget
class auto_completion_model: public ro_tree_view_model {
public:
    /// Constructor
    explicit auto_completion_model(cli::app_command_interpreter & interp);

    /// Destructor
    ~auto_completion_model() override = default;

    /// Returns columns size
    std::size_t columns_size() const override;

    /// Returns column name for index
    std::wstring column_name(std::size_t index) const override;

    /// Returns childs size
    std::size_t childs_size(const row_index & row) const override;

    /// Returns child for index
    row_index child(const row_index & row, std::size_t index) const override;

    /// Returns index in parent
    size_t index(const row_index & row) const override;

    /// Returns parent index
    row_index parent(const row_index & row) const override;

    /// Returns text for index
    std::wstring text(const row_index & row, std::size_t c) const override;

    /// Safely connects prefix changed signal to event processor in data
    CXXDBG_DEFINE_SIGNALX(handle_prefix_changed, void(const std::string &))

private:
     auto_completion_data data_;    ///< Model data
};

}

