// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file global_variable.cpp
/// Contains implementation of the global_variable class.

#include "global_variable.hpp"
#include "target_base.hpp"
#include "utils.hpp"
#include <lldb/API/SBDeclaration.h>
#include <lldb/Symbol/Type.h>
#include <lldb/Symbol/Variable.h>


namespace cxxdbg::dbg::core {


global_variable::global_variable(target_base & targ, lldb_private::Variable * var, uint64_t a):
targ_{targ},
var_{var},
addr_{a} {
}


std::string global_variable::name() const {
    auto name_cstr = var_->GetName().AsCString();
    if (name_cstr) {
        return name_cstr;
    }

    return {};
}


type_impl * global_variable::type() const {
    auto comp_type = var_->GetType()->GetForwardCompilerType();
    lldb::SBType sb_type{comp_type};
    return targ_.get_or_create_type(sb_type);
}


dbgfmt::backend::source_position global_variable::def_pos() const {
    auto decl = var_->GetDeclaration();
    if (!decl.IsValid()) {
        return {};
    }

    auto file = lldb_file_spec_to_path(decl.GetFile());
    return {file, static_cast<unsigned int>(decl.GetLine())};
}


}
