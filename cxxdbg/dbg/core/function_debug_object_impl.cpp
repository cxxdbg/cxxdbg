// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file function_debug_object_impl.cpp
/// Contains implementation of the function_debug_object_class.

#include "function_debug_object_impl.hpp"
#include "record_type_impl.hpp"
#include "target_base.hpp"
#include <clang/AST/DeclCXX.h>
#include <lldb/Symbol/Function.h>
#include <Plugins/TypeSystem/Clang/TypeSystemClang.h>


namespace cxxdbg::dbg::core {


uint64_t function_debug_object_impl::addr() const {
    auto addr = func().GetStartAddress();
    return static_cast<uint64_t>(addr.GetLoadAddress(targ_.lldb_targ()));
}


std::string function_debug_object_impl::name() const {
    auto name_cstr = func().GetName();
    if (name_cstr) {
        return name_cstr;
    }

    return {};
}


dbgfmt::backend::type_impl * function_debug_object_impl::type() const {
    auto type = func().GetType();
    assert(type.IsValid() && "invalid lldb function type");
    return targ_.get_or_create_type(type);
}


dbgfmt::backend::source_position function_debug_object_impl::def_pos() const {
    auto addr = func().GetStartAddress();
    auto line_entry = addr.GetLineEntry();
    if (!line_entry.IsValid()) {
        return {};
    }

    auto pos = source_position::from_line_entry(line_entry);
    return {pos.path(), pos.line()};
}


dbgfmt::backend::record_type_impl * function_debug_object_impl::record_type() const {
    // getting declaration context for function
    auto func_decl = func().get()->GetDeclContext();
    if (!func_decl) {
        return nullptr;
    }

    // checking function is c/c++
    auto type_system = func_decl.GetTypeSystem();
    if (!llvm::dyn_cast<lldb_private::TypeSystemClang>(type_system)) {
        return nullptr;
    }

    // checking that function is method

    clang::DeclContext * dctx = (clang::DeclContext*)(func_decl.GetOpaqueDeclContext());
    if (!dctx) {
        return nullptr;
    }

    clang::CXXMethodDecl * mdecl = llvm::dyn_cast<clang::CXXMethodDecl>(dctx);
    if (!mdecl) {
        return nullptr;
    }

    // getting parent record of method
    clang::DeclContext * record_dctx = mdecl->getDeclContext();
    auto record_decl = llvm::dyn_cast<clang::CXXRecordDecl>(record_dctx);
    if (!record_decl) {
        return nullptr;
    }

    // converting record decl to opaque decl for clang type system. An opaque decl
    // should always point to the base Decl class
    auto opaque_record_decl = static_cast<clang::Decl*>(record_decl);

    // creating compiler type
    auto compiler_type = type_system->GetTypeForDecl(opaque_record_decl);
    if (!compiler_type) {
        return nullptr;
    }

    // creating SBType instance from compiler type
    lldb::SBType sb_type{compiler_type};

    // creating type impl from SBType and converting it to record type
    auto type = targ_.get_or_create_type(sb_type);
    assert(type && "get_or_create_type returned null");
    auto rtype = dynamic_cast<record_type_impl*>(type);
    assert(rtype && "get_or_create_type returned not a record");
    return rtype;
}


}
