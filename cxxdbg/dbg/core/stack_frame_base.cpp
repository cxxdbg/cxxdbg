// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file stack_frame_base.cpp
/// Contains implementation of stack_frame_base class.

#include "stack_frame_base.hpp"
#include "lldb_value_storage.hpp"
#include "target_base.hpp"
#include "variable.hpp"
#include "cxxdbg/log/log.hpp"
#include <lldb/API/SBDeclaration.h>
#include <lldb/API/SBExpressionOptions.h>
#include <lldb/Target/StackFrame.h>
#include <lldb/Target/Thread.h>
#include <lldb/Symbol/VariableList.h>
#include <lldb/Symbol/VariableList.h>
#include <lldb/ValueObject/ValueObject.h>


namespace cxxdbg::dbg::core {



call_parameter::call_parameter(const lldb::SBValue & v):
val_(v) {
    assert(val().IsValid() && "Invalid call parameter");
}


call_parameter::call_parameter(const call_parameter & par):
val_(par.val_) {
}


call_parameter::~call_parameter() {
}


std::string call_parameter::name() const {
    const char * nm = val().GetName();
    return nm != nullptr ? std::string(nm) : std::string();
}


std::string call_parameter::type() const {
    const char * tp = val().GetTypeName();
    return tp != nullptr ? std::string(tp) : std::string();
}


std::string call_parameter::value() const {
    const char *  v = val().GetValue();
    return v != nullptr ? std::string(v) : std::string();
}


lldb::SBValue call_parameter::val() const {
    return val_;
}



stack_frame_base::stack_frame_base(const thread_base & thrd, size_t idx):
targ_{thrd.targ()},
frame_{thrd.thrd().GetSP()->GetStackFrameAtIndex(static_cast<uint32_t>(idx))} {
}


stack_frame_base::~stack_frame_base() {
}


code_position stack_frame_base::read_pos() const {
    lldb::SBAddress addr{frame_->GetFrameCodeAddress()};
    return code_position{targ_, addr};
}


uint64_t stack_frame_base::get_cfa() const {
    return static_cast<uint64_t>(frame_->GetStackID().GetCallFrameAddress());
}


uint64_t stack_frame_base::get_sc_id() const {
    auto scope = frame_->GetStackID().GetSymbolContextScope();
    return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(scope));
}


std::vector<call_parameter> stack_frame_base::read_params() const {
    std::vector<call_parameter> res;

    lldb_private::Status status;
    lldb_private::VariableList * vars = frame_->GetVariableList(false, &status);
    if (vars == nullptr)
        return res;

    res.reserve(vars->GetSize());

    for (uint32_t i = 0, e = vars->GetSize(); i < e; ++i) {
        lldb::VariableSP var = vars->GetVariableAtIndex(i);
        if (var->GetScope() != lldb::eValueTypeVariableArgument)
            continue;

        lldb::ValueObjectSP val =
                frame_->GetValueObjectForFrameVariable(var, lldb::eDynamicDontRunTarget);
        lldb::SBValue sbval;
        sbval.SetSP(val);
        res.push_back(call_parameter{sbval});
    }

    return res;
}


std::vector<const variable*> stack_frame_base::read_params_vars() const {
    std::vector<const variable*> res;

    lldb_private::Status status;
    lldb_private::VariableList * vars = frame_->GetVariableList(false, &status);
    if (vars == nullptr)
        return res;

    res.reserve(vars->GetSize());

    for (uint32_t i = 0, e = vars->GetSize(); i < e; ++i) {
        lldb::VariableSP var = vars->GetVariableAtIndex(i);
        if (var->GetScope() != lldb::eValueTypeVariableArgument)
            continue;

        res.push_back(targ_.get_or_create_var(var));
    }

    return res;
}


std::vector<const variable*> stack_frame_base::read_locals(bool params) const {
    std::vector<const variable*> res;

    lldb_private::Status status;
    lldb_private::VariableList * vars = frame_->GetVariableList(false, &status);
    if (vars == nullptr)
        return res;

    res.reserve(vars->GetSize());

    for (uint32_t i = 0, e = vars->GetSize(); i < e; ++i) {
        lldb::VariableSP var = vars->GetVariableAtIndex(i);
        if (var->GetScope() == lldb::eValueTypeVariableGlobal ||
            var->GetScope() == lldb::eValueTypeVariableStatic)
            continue;

        if (!var->IsInScope(frame_.get()))
            continue;

        if (!params && var->GetScope() == lldb::eValueTypeVariableArgument) {
            continue;
        }

        res.push_back(targ_.get_or_create_var(var));
    }

    return res;
}


lldb::SBValue stack_frame_base::read_var_lldb_value(const variable * var) {
    lldb::ValueObjectSP raw_val = frame_->GetValueObjectForFrameVariable(var->lldb_var(), lldb::eDynamicDontRunTarget);
    lldb::SBValue val;
    val.SetSP(raw_val);
    return val;
}


type_impl * stack_frame_base::read_var_type(const variable * var) {
    auto val = read_var_lldb_value(var);
    auto lldb_type = val.GetType();
    assert(lldb_type.IsValid() && "invalid LLDB value type");
    auto type = targ_.get_or_create_type(lldb_type);
    assert(type != nullptr && "get_or_create_type returned nullptr");
    return type;
}


uint64_t stack_frame_base::read_var_addr(const variable * var) {
    auto val = read_var_lldb_value(var);

    // val.GetLoadAddress() sometimes may return value of pointer itself
    // for pointer values (like this), not address of pointer value. We have to check
    // value address type to avoid this.

    const bool scalar_is_load_address = false;      // return invalid address for scalars
    lldb_private::AddressType addr_type;
    val.GetSP()->GetAddressOf(scalar_is_load_address, &addr_type);
    if (addr_type == lldb_private::eAddressTypeLoad || addr_type == lldb_private::eAddressTypeFile) {
        // trying get address of value
        return val.GetLoadAddress();
    }

    return UINT64_MAX;
}


bool stack_frame_base::is_valid() const {
    return frame_.get() != nullptr;
}


static const char * str_or_null(const char * s) {
    if (s == nullptr) {
        return "(null)";
    }

    return s;
}


std::shared_ptr<dbgfmt::data::storage> stack_frame_base::get_var_lldb_value_storage(const variable * var) {
    auto val = read_var_lldb_value(var);
    return std::make_shared<lldb_value_storage>(val);
}


std::tuple<std::shared_ptr<dbgfmt::data::storage>, type_impl*>
stack_frame_base::do_eval_expr(const std::string & expr) const {

    lldb::SBExpressionOptions opts;
    auto eval_res = lldb::SBFrame{frame_}.EvaluateExpression(expr.c_str(), opts);
    if (!eval_res.IsValid()) {
        throw dbgfmt::backend::format_error{"invalid lldb value"};
    }

    // getting lldb value type
    lldb::SBType lldb_type = eval_res.GetType();
    if (!lldb_type.IsValid()) {
        // trying get error from value
        lldb::SBError lerr = eval_res.GetError();
        if (lerr.IsValid() && lerr.Fail() && lerr.GetCString() != nullptr) {
            // cutting error message after end of line
            const char * begin = lerr.GetCString();
            const char * end = begin;
            while (*end != 0 && *end != '\n') {
                ++end;
            }

            std::string msg{begin, end};
            throw dbgfmt::backend::format_error{msg};
        }

        throw dbgfmt::backend::format_error{"can't get lldb value type"};
    }

    auto type = targ_.get_or_create_type(lldb_type);
    assert(type != nullptr && "get_or_create_type returned nullptr");

    // creating and returning storage for lldb value
    return std::make_tuple(std::make_shared<lldb_value_storage>(eval_res), type);
}


registers_list_base stack_frame_base::read_registers_base() const {
    lldb::SBFrame sbframe{frame()};
    return registers_list_base{targ_, sbframe.GetRegisters()};
}


lldb::StackFrameSP stack_frame_base::frame() const {
    return frame_;
}


}
