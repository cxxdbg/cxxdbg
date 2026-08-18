// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file type_utils.cpp
/// Contains implementation of utility functions for working with
/// lldb types.

#include "type_utils.hpp"
#include "cxxdbg/util/template.hpp"
#include <lldb/API/SBTarget.h>
#include <lldb/API/SBValue.h>
#include <Plugins/TypeSystem/Clang/TypeSystemClang.h>
#include <lldb/Symbol/Function.h>
#include <lldb/Symbol/Type.h>
#include <clang/AST/DeclCXX.h>
#include <llvm/Support/Casting.h>
#include <sstream>
#include <vector>


namespace cxxdbg::dbg::core {


bool lldb_type_is_record(lldb::SBType type) {
    assert(type.IsValid() && "invalid lldb type");
    auto cls = type.GetTypeClass();
    return cls == lldb::eTypeClassClass ||
           cls == lldb::eTypeClassStruct ||
           cls == lldb::eTypeClassUnion;
}


lldb::SBType lldb_type_pointee(lldb::SBType type) {
    assert(type.IsValid() && "invalid lldb type");

    if (type.GetTypeClass() == lldb::eTypeClassReference) {
        return type.GetDereferencedType();
    }

    assert(type.GetTypeClass() == lldb::eTypeClassPointer &&
           "lldb type is not pointer or reference");
    return type.GetPointeeType();
}


std::string lldb_type_name(lldb::SBType type) {
    assert(type.IsValid() && "invalid lldb type");

    const char * nm = type.GetUnqualifiedType().GetCanonicalType().GetName();
    assert(nm != nullptr && "Can't get lldb type name");

    return nm;
}


std::string lldb_type_qual_name(lldb::SBType type) {
    assert(type.IsValid() && "invalid lldb type");

    const char * nm = type.GetCanonicalType().GetName();
    assert(nm != nullptr && "Can't get lldb type name");

    return nm;
}


std::string lldb_type_template_name(lldb::SBType type) {
    std::istringstream str(lldb_type_name(type));
    std::string name;
    std::vector<std::string> pars;
    util::parse_template_name(str, name, pars);
    return name;
}


size_t lldb_type_template_params_size(lldb::SBType type) {
    std::istringstream str(lldb_type_name(type));
    std::string name;
    std::vector<std::string> pars;
    util::parse_template_name(str, name, pars);
    return pars.size();
}


std::string lldb_type_template_param(lldb::SBType type, size_t index) {
    const auto & tname = lldb_type_name(type);
    std::istringstream str(tname);
    std::string name;
    std::vector<std::string> pars;
    util::parse_template_name(str, name, pars);

    if (index >= pars.size()) {
        return {};
    }

    return pars[index];
}


/// Returns clang record decl for lldb type.
/// Returns nullptr if record type can't be retrieved
clang::CXXRecordDecl * get_record_decl_for_type(lldb::SBType type) {
    assert(type.IsValid() && "invalid lldb type");

    auto compiler_type = type.GetSP()->GetCompilerType(true);
    assert(compiler_type.IsValid() && "compiler type for record is invalid");

    // checking that type is a clang type
    auto type_system = compiler_type.GetTypeSystem();
    auto clang_type_system = type_system.dyn_cast_or_null<lldb_private::TypeSystemClang>();
    if (!clang_type_system)
        return nullptr;

    // force LLDB to load complete debug info for this type
    compiler_type.GetCompleteType();

    // getting record decl
    auto oct = compiler_type.GetOpaqueQualType();
    return clang_type_system->GetAsCXXRecordDecl(oct);
}


std::vector<std::string> lldb_type_mem_funcs(lldb::SBType type) {
    auto rec_type = get_record_decl_for_type(type);
    if (!rec_type || !rec_type->hasDefinition()) {
        return{};
    }

    std::vector<std::string> res;
    for(clang::Decl * decl : rec_type->decls()) {
        auto mdecl = llvm::dyn_cast<clang::CXXMethodDecl>(decl);
        if (!mdecl) {
            continue;
        }

        res.push_back(mdecl->getNameAsString());
    }

    return res;
}


lldb::SBType untypedef_type(lldb::SBType type) {
    auto res = type;
    while (res.IsTypedefType()) {
        res = res.GetTypedefedType();
    }

    return res;
}


}
