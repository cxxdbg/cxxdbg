// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file mem_ptr_type_impl.cpp
/// Contains implementation of the mem_ptr_type_impl class.

#include "mem_ptr_type_impl.hpp"
#include "target_base.hpp"
#include "dbgfmt/backend/format_error.hpp"
#include <lldb/Symbol/CompilerType.h>


namespace cxxdbg::dbg::core {


mem_ptr_type_impl::mem_ptr_type_impl(target_base & targ, const lldb::SBType & t):
core::type_impl{targ, t} {
    auto tp = lldb_type();

    if (tp.GetTypeClass() != lldb::eTypeClassMemberPointer) {
        throw dbgfmt::backend::format_error{"lldb value type is not a pointer to member"};
    }
}


const type_impl * mem_ptr_type_impl::mem_type() const {
    // getting compiler type
    auto compiler_type = lldb_type().GetSP()->GetCompilerType(true);

    // checking that type is a clang type
    auto type_system = compiler_type.GetTypeSystem();
    auto clang_type_system = type_system.dyn_cast_or_null<lldb_private::TypeSystemClang>();
    if (!clang_type_system) {
        throw dbgfmt::backend::format_error{"compiler type is not a clang type"};
    }

    // getting type of pointee
    auto oct = compiler_type.GetOpaqueQualType();
    auto mem_ptr_type = reinterpret_cast<clang::MemberPointerType*>(oct);
    auto ptype = mem_ptr_type->getPointeeType();

    // creating compiler type, SBType and type_impl for pointee type
    lldb_private::CompilerType c_ptype{type_system, ptype.getAsOpaquePtr()};
    lldb::SBType lldb_ptype{c_ptype};
    return targ().get_or_create_type(lldb_ptype);
}


const record_type_impl * mem_ptr_type_impl::obj_type() const {
    // getting compiler type
    auto compiler_type = lldb_type().GetSP()->GetCompilerType(true);

    // checking that type is a clang type
    auto type_system = compiler_type.GetTypeSystem();
    auto clang_type_system = type_system.dyn_cast_or_null<lldb_private::TypeSystemClang>();
    if (!clang_type_system) {
        throw dbgfmt::backend::format_error{"compiler type is not a clang type"};
    }

    // getting type of record
    auto oct = compiler_type.GetOpaqueQualType();
    auto mem_ptr_type = reinterpret_cast<clang::MemberPointerType*>(oct);
    auto rtype = mem_ptr_type->getClass();

    // creating compiler type, SBType and type_impl for record type
    lldb_private::CompilerType c_rtype{type_system, clang::QualType{rtype, 0}.getAsOpaquePtr()};
    lldb::SBType lldb_rtype{c_rtype};
    auto res = targ().get_or_create_type(lldb_rtype);
    auto rtype_res = dynamic_cast<record_type_impl*>(res);
    assert(res && "returned type is not a record");
    return rtype_res;
}


}
