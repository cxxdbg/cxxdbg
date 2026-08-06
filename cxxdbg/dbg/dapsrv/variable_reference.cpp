// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file variable_reference.cpp
/// Contains implementations of classes for managing variable references in CXXDBG DAP server

#include "variable_reference.hpp"
#include "cxxdbg/dbg/core/target.hpp"
#include "cxxdbg/dbg/core/variable.hpp"
#include <sstream>


namespace cxxdbg::dbg::dapsrv {


size_t frame_variable_reference::vars_size() const {
    return frame_.read_locals().size();
}


dbgfmt::named_fmt_result frame_variable_reference::var_at(size_t idx) {
    auto locals = frame_.read_locals();
    if (idx >= locals.size()) {
        throw std::runtime_error{"variable index in frame is invalid"};
    }

    auto var = locals[idx];
    auto val = frame_.read_var_value(var);
    auto fres = var_tab().targ().format_value(val);
    auto def_pos = var->def_pos();
    auto var_name = var->name();
    return {std::wstring{var_name.begin(), var_name.end()}, {def_pos.path(), def_pos.line()}, fres};
}


size_t registers_variable_reference::vars_size() const {
    return frm_.read_registers().groups_size();
}


dbgfmt::named_fmt_result registers_variable_reference::var_at(size_t idx) {
    auto registers = frm_.read_registers();
    if (idx >= registers.groups_size()) {
        throw std::runtime_error{"register group index in frame is invalid"};
    }

    return registers.group_fmt_result_at(idx);
}


size_t fmt_result_variable_reference::vars_size() const {
    return fres_.childs_size();
}


dbgfmt::named_fmt_result fmt_result_variable_reference::var_at(size_t idx) {
    if (idx >= fres_.childs_size()) {
        throw std::runtime_error{"child variable index is invalid"};
    }

    return fres_.child_at(idx);
}


void variable_reference_table::clear() {
    var_refs_.clear();
    local_var_refs_.clear();
    registers_var_refs_.clear();
//    param_var_refs_.clear();
    fmt_result_refs_.clear();
    next_var_ref_id_ = 1;
}


variable_reference * variable_reference_table::var_ref(size_t id) {
    auto it = var_refs_.find(id);
    if (it != var_refs_.end()) {
        return it->second.get();
    }

    std::ostringstream msg;
    msg << "Variable reference " << id << " not found";
    throw std::runtime_error{msg.str()};
}


variable_reference * variable_reference_table::locals_ref(const core::stack_frame & frm) {
    auto it = local_var_refs_.find(frm);
    if (it != local_var_refs_.end()) {
        return it->second;
    }

    auto new_ref = std::make_unique<locals_variable_reference>(*this, next_var_ref_id_++, frm);
    auto res = new_ref.get();
    var_refs_.emplace(res->id(), std::move(new_ref));
    local_var_refs_.emplace(frm, res);
    return res;
}


variable_reference * variable_reference_table::registers_ref(const core::stack_frame & frm) {
    auto it = registers_var_refs_.find(frm);
    if (it != registers_var_refs_.end()) {
        return it->second;
    }

    auto new_ref = std::make_unique<registers_variable_reference>(*this, next_var_ref_id_++, frm);
    auto res = new_ref.get();
    var_refs_.emplace(res->id(), std::move(new_ref));
    registers_var_refs_.emplace(frm, res);
    return res;
}


variable_reference * variable_reference_table::fmt_result_ref(const dbgfmt::fmt_result_ref & fres) {
    auto it = fmt_result_refs_.find(fres.get());
    if (it != fmt_result_refs_.end()) {
        return it->second;
    }

    auto new_ref = std::make_unique<fmt_result_variable_reference>(*this, next_var_ref_id_++, fres);
    auto res = new_ref.get();
    var_refs_.emplace(res->id(), std::move(new_ref));
    fmt_result_refs_.emplace(fres.get(), res);
    return res;
}


}
