// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "auto_completion_model.hpp"
#include "application.hpp"

#include "cxxdbg/cli/app_command_interpreter.hpp"


namespace cxxdbg::dbg {


auto_completion_data::auto_completion_data(cli::app_command_interpreter & interp,
                                           ro_tree_view_model & model) :
interp_(interp) {
    before_added().connect(model.before_added());
    after_added().connect(model.after_added());
    before_removed().connect(model.before_removed());
    after_removed().connect(model.after_removed());
}


void auto_completion_data::handle_prefix_changed(const std::string & prefix) {
    size_t sz = size();

    before_removed()({}, 0, sz);
    commands_.clear();
    after_removed()({}, 0, sz);

    auto entries = interp_.available_commands(prefix);

    sz = entries.size();
    before_added()({}, 0, sz);

    for (auto & entry : entries) {
        commands_.emplace_back(std::wstring{entry.begin(), entry.end()}); // TODO: implement proper convertion
    }

    after_added()({}, 0, sz);
}


auto_completion_model::auto_completion_model(cli::app_command_interpreter & interp):
data_(interp, dynamic_cast<ro_tree_view_model &>(*this)) {

    handle_prefix_changed().connect([this](const std::string & prefix) {
        this->data_.handle_prefix_changed(prefix);
    });
}


std::size_t auto_completion_model::columns_size() const {
    return 1;
}


std::wstring auto_completion_model::column_name(std::size_t index) const {
    if (index == 0) {
        return L"available commands";
    }

    return {};
}


std::size_t auto_completion_model::childs_size(const ro_tree_view_model::row_index & row) const {
    return row.is_valid() ? 0 : data_.size();
}


ro_tree_view_model::row_index
auto_completion_model::child(const ro_tree_view_model::row_index & row, std::size_t index) const {
    if (!row) {
        return make_index(index + 1);
    }

    return {};
}


std::size_t auto_completion_model::index(const ro_tree_view_model::row_index & row) const {
    if (!row)
        return 0;

    auto index = static_cast<size_t>(row.val());
    assert(index > 0);

    return index - 1;
}


ro_tree_view_model::row_index auto_completion_model::parent(const ro_tree_view_model::row_index & row) const {
    return {};
}


std::wstring auto_completion_model::text(const ro_tree_view_model::row_index & row, std::size_t c) const {
    if (c > 0 || !row)
        return {};

    auto index = static_cast<size_t>(row.val());
    assert(index > 0 && index <= data_.size());

    return data_.child_at(index - 1);
}

}
