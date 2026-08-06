// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file variable.cpp
/// Contains implementation of variable class.

#include "variable.hpp"
#include "utils.hpp"
#include <lldb/Symbol/Variable.h>
#include <iostream>


namespace cxxdbg::dbg::core {


variable::variable(lldb::VariableSP v):
var_{std::move(v)} {
}


variable::~variable() {
}


std::string variable::name() const {
    const char * nm = var_->GetName().GetCString();
    return nm ? std::string(nm) : std::string();
}


source_position variable::def_pos() const {
    auto decl = var_->GetDeclaration();
    if (!decl.IsValid()) {
        return {};
    }

    auto file = lldb_file_spec_to_path(decl.GetFile());
    return {file, static_cast<unsigned int>(decl.GetLine())};
}


lldb::VariableSP variable::lldb_var() const {
    return var_;
}


unsigned long variable::id() const {
    return static_cast<unsigned long>(var_->GetID());
}


}
