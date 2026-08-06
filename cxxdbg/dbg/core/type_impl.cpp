// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file type_impl.cpp
/// Contains implementation of the type_impl class.

#include "type_impl.hpp"
#include "target_base.hpp"
#include "type_utils.hpp"
#include "utils.hpp"
#include "dbgfmt/backend/format_error.hpp"
#include <lldb/API/SBProcess.h>
#include <lldb/Symbol/CompilerType.h>
#include <lldb/Symbol/Type.h>
#include <lldb/Target/LanguageRuntime.h>


namespace cxxdbg::dbg::core {


type_impl::type_impl(target_base & targ, const lldb::SBType & t):
targ_{targ},
lldb_type_{t} {
    if (!lldb_type().IsValid()) {
        throw dbgfmt::backend::format_error{"invalid lldb pointer type"};
    }
}


dbgfmt::backend::type_context_impl * type_impl::ctx() const {
    return &targ_;
}


std::string type_impl::name() const {
    return lldb_type_name(lldb_type());
}


dbgfmt::backend::source_position type_impl::def_pos() const {
    auto type_impl = lldb_type().GetSP();
    if (!type_impl) {
        return {};
    }

    auto compiler_type = type_impl->GetCompilerType(true);
    assert(compiler_type.IsValid() && "compiler type for record is invalid");

    auto type = compiler_type.GetDebuggerType();
    if (!type) {
        return {};
    }

    auto & decl = type->GetDeclaration();
    if (!decl.IsValid()) {
        return {};
    }

    auto file = lldb_file_spec_to_path(decl.GetFile());
    if (file.empty()) {
        return {};
    }

    return {file, decl.GetLine()};
}


std::string type_impl::qual_name() const {
    return lldb_type_qual_name(lldb_type());
}


const type_impl * type_impl::pointer_type() const {
    return targ_.get_or_create_ptr_type(this);
}


size_t type_impl::size() const {
    return static_cast<size_t>(lldb_type().GetByteSize());
}


const type_impl * type_impl::dynamic_type_at_addr(uint64_t & addr) const {
    auto ctype = lldb_type().GetSP()->GetCompilerType(false);

    lldb_private::ExecutionContext exe_ctx{targ_.lldb_targ().GetSP(), true};

    // creating lldb ValueObject for specified address value
    // TODO: check if we want always use eByteOrderLittle byte order
    lldb_private::DataExtractor data;
    auto addr_32 = static_cast<uint32_t>(addr);
    if (targ_.lldb_targ().GetAddressByteSize() == 8) {
        data = lldb_private::DataExtractor{&addr, sizeof(addr), lldb::eByteOrderLittle, 8};
    } else if (targ_.lldb_targ().GetAddressByteSize() == 4) {
        data = lldb_private::DataExtractor{&addr_32, sizeof(addr_32), lldb::eByteOrderLittle, 4};
    } else {
        assert(false && "Unknown address byte size");
        return this;
    }

    auto val_obj = lldb_private::ValueObject::CreateValueObjectFromData("addr_obj", data, exe_ctx, ctype);
    assert(val_obj && "can't create value object at specified address");

    auto process = targ_.lldb_targ().GetSP()->GetProcessSP();

    // NOTE: all following code is rewriten from
    // the ValueObjectDynamicValue::UpdateValue method from LLDB

    lldb_private::TypeAndOrName class_type_or_name;
    lldb_private::Address dynamic_address;
    bool found_dynamic_type = false;
    lldb_private::Value::ValueType value_type;

    lldb_private::LanguageRuntime *runtime = nullptr;

    //lldb::LanguageType known_type = m_parent->GetObjectRuntimeLanguage();
    lldb::LanguageType known_type = ctype.GetMinimumLanguage();
    if (known_type != lldb::eLanguageTypeUnknown &&
        known_type != lldb::eLanguageTypeC) {
      runtime = process->GetLanguageRuntime(known_type);
      if (runtime)
        found_dynamic_type = runtime->GetDynamicTypeAndAddress(
            *val_obj, lldb::eDynamicDontRunTarget, class_type_or_name, dynamic_address,
            value_type);
    } else {
      runtime = process->GetLanguageRuntime(lldb::eLanguageTypeC_plus_plus);
      if (runtime)
        found_dynamic_type = runtime->GetDynamicTypeAndAddress(
            *val_obj, lldb::eDynamicDontRunTarget, class_type_or_name, dynamic_address,
            value_type);

      if (!found_dynamic_type) {
        runtime = process->GetLanguageRuntime(lldb::eLanguageTypeObjC);
        if (runtime)
          found_dynamic_type = runtime->GetDynamicTypeAndAddress(
              *val_obj, lldb::eDynamicDontRunTarget, class_type_or_name, dynamic_address,
              value_type);
      }
    }

    if (runtime && found_dynamic_type) {
      if (class_type_or_name.HasType()) {
        auto res_t_name = runtime->FixUpDynamicType(class_type_or_name, *val_obj);
        auto res_ctype = res_t_name.GetCompilerType();
        addr = dynamic_address.GetLoadAddress(targ().lldb_targ().GetSP().get());
        return targ_.get_or_create_type(lldb::SBType{res_ctype});
      }
    }

    return this;
}


lldb::SBType type_impl::lldb_type() const {
    return lldb_type_;
}


}
