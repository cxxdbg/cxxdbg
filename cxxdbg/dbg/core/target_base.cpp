// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file target_base.cpp
/// Contains implementation of target_base class.

#include "target_base.hpp"
#include "address_breakpoint.hpp"
#include "array_type_impl.hpp"
#include "builtin_type_impl.hpp"
#include "code_model.hpp"
#include "debugger_base.hpp"
#include "enum_type_impl.hpp"
#include "function_breakpoint.hpp"
#include "function_type_impl.hpp"
#include "lldb_disassembler.hpp"
#include "lldb_pointer_type_impl.hpp"
#include "lldb_value_storage.hpp"
#include "log.hpp"
#include "mem_ptr_type_impl.hpp"
#include "memory_disassembler.hpp"
#include "record_type_impl.hpp"
#include "source_position_breakpoint.hpp"
#include "typedef_type_impl.hpp"
#include "variable.hpp"
#include "vector_type_impl.hpp"
#include "utils.hpp"

#include <lldb/API/SBBreakpoint.h>
#include <lldb/API/SBDebugger.h>
#include <lldb/API/SBEvent.h>
#include <lldb/API/SBExpressionOptions.h>
#include <lldb/API/SBInstruction.h>
#include <lldb/API/SBListener.h>
#include <lldb/API/SBProcess.h>
#include <lldb/API/SBTarget.h>
#include <lldb/API/SBThread.h>
#include <lldb/Symbol/Variable.h>
#include <lldb/Target/Target.h>

#include <iostream>
#include <sstream>
#include <filesystem>


namespace fs = std::filesystem;


namespace cxxdbg::dbg::core {


bool target_base::compare_lldb_func::operator()(const lldb::SBFunction & f1,
                                           const lldb::SBFunction & f2) const {
    auto f1_copy = f1;
    auto f2_copy = f2;
    return f1_copy.get() < f2_copy.get();
}


bool target_base::compare_lldb_sym::operator()(const lldb::SBSymbol & s1,
                                          const lldb::SBSymbol & s2) const {
    auto s1_copy = s1;
    auto s2_copy = s2;
    return s1_copy.GetStartAddress().GetLoadAddress(targ_) <
           s2_copy.GetStartAddress().GetLoadAddress(targ_);
}


/// Converts lldb state to state
static target_base::state_t lldb_state_to_state(lldb::StateType st) {
    switch (st) {
    case lldb::eStateLaunching:
    case lldb::eStateAttaching:
    case lldb::eStateConnected:
        return target_base::state_t::launching;
    case lldb::eStateRunning:
        return target_base::state_t::running;
    case lldb::eStateStopped:
    case lldb::eStateCrashed:
        return target_base::state_t::stopped;
    case lldb::eStateExited:
    case lldb::eStateDetached:
        return target_base::state_t::loaded;
    default:
        assert(false && "Unknonw LLDB state");
        return target_base::state_t::loaded;
    }
}


std::string target_base::state_name(state_t st) {
    switch (st) {
    case state_t::loaded:       return "loaded";
    case state_t::launching:    return "launching";
    case state_t::running:      return "running";
    case state_t::stopped:      return "stopped";
    default:
        assert(false && "unknown state");
        return "unknown";
    }
}


target_base::target_base(debugger_base & dbg, const std::filesystem::path & exe_path):
target_base(dbg, dbg.do_load_target(exe_path)) {
}


target_base::target_base(debugger_base & dbg, unsigned long pid):
target_base(dbg, dbg.do_attach(pid)) {
}


target_base::target_base(debugger_base & dbg, const std::string & name):
target_base(dbg, dbg.do_attach(name)) {
}


target_base::target_base(debugger_base & dbg):
target_base(dbg, dbg.do_get_dummy_target()) {
}


target_base::~target_base() {
    if (targ_ != targ_.GetDebugger().GetDummyTarget()) {
        bool res = targ_.GetDebugger().DeleteTarget(targ_);
        assert(res && "Can't delete LLDB target");
    }
}


lldb::SBTarget target_base::lldb_targ() const {
    return targ_;
}


size_t target_base::ptr_byte_size() const {
    return static_cast<size_t>(lldb_targ().GetAddressByteSize());
}


std::size_t target_base::modules_size() const {
    return static_cast<std::size_t>(lldb_targ().GetNumModules());
}


module target_base::module_at(std::size_t i) const {
    lldb::SBTarget t = targ_;
    return module(t.GetModuleAtIndex(static_cast<uint32_t>(i)));
}


target_base::const_module_iterator target_base::modules_begin() const {
    return const_module_iterator(*this, 0);
}


target_base::const_module_iterator target_base::modules_end() const {
    return const_module_iterator(*this, modules_size());
}


void target_base::reindex() {
    for (auto it = modules_begin(), end = modules_end(); it != end; ++it) {
        it->reindex();
    }
}


code_position target_base::find_func(const std::string & name) {
    lldb::SBSymbolContextList res = lldb_targ().FindFunctions(name.c_str());
    for (int i = 0, e = res.GetSize(); i != e; ++i) {
        lldb::SBSymbolContext ctx = res.GetContextAtIndex(i);
        if (!ctx.IsValid())
            continue;

        lldb::SBFunction func = ctx.GetFunction();
        if (!func.IsValid())
            continue;

        if (func.GetName() == nullptr)
            continue;

        if (name != func.GetName())
            continue;

        return {*this, func.GetStartAddress()};
    }

    return {*this};
}


lldb::SBTypeList target_base::find_types(const lldb::SBTarget & targ, const std::string & name) {
    // converting type name to canonical forms
    std::string gcc_can_name;
    std::string clang_can_name;
    if (!dbg_.cm().make_canonical_type_names(name, gcc_can_name, clang_can_name)) {
        // can't parse type name, use specified name as canonical
        std::cerr << "can't parse type name '" << name << "'\n";
        gcc_can_name = name;
        clang_can_name = name;
    }

    lldb::SBTarget t = targ;

    // looking for types using gcc canonical name
    auto res = t.FindTypes(gcc_can_name.c_str());
    if (res.GetSize() != 0 || gcc_can_name == clang_can_name) {
        return res;
    }

    // looking for types using clang canonical nam
    return t.FindTypes(clang_can_name.c_str());
}


type_impl * target_base::find_type(const std::string & nm) {

    // trying find type in cache
    auto res = find_types_.find(nm);
    if (res != find_types_.end()) {
        return res->second;
    }

    // looking for types in lldb
    auto types = find_types(lldb_targ(), nm);

    if (types.GetSize() == 0) {
        // type not found
        return nullptr;
    }

    // adding first type to cache and returning it
    auto tp = types.GetTypeAtIndex(0);
    auto new_type = get_or_create_type(tp);
    auto ins_res = find_types_.emplace(nm, new_type);
    assert(ins_res.second && "type should not exist");

    return new_type;
}


/// Returns LLDB basic type class from fmt builtin type kind
static lldb::BasicType get_lldb_basic_type(dbgfmt::backend::builtin_type_impl::kind_t kind) {
    switch (kind) {
    case builtin_type_impl::kind_t::void_:
        return lldb::eBasicTypeVoid;
    case builtin_type_impl::kind_t::bool_:
        return lldb::eBasicTypeBool;
    case builtin_type_impl::kind_t::char_:
        return lldb::eBasicTypeChar;
    case builtin_type_impl::kind_t::short_:
        return lldb::eBasicTypeShort;
    case builtin_type_impl::kind_t::int_:
        return lldb::eBasicTypeInt;
    case builtin_type_impl::kind_t::long_:
        return lldb::eBasicTypeLong;
    case builtin_type_impl::kind_t::long_long_:
        return lldb::eBasicTypeLongLong;
    case builtin_type_impl::kind_t::unsigned_char_:
        return lldb::eBasicTypeUnsignedChar;
    case builtin_type_impl::kind_t::unsigned_short_:
        return lldb::eBasicTypeUnsignedShort;
    case builtin_type_impl::kind_t::unsigned_int_:
        return lldb::eBasicTypeUnsignedInt;
    case builtin_type_impl::kind_t::unsigned_long_:
        return lldb::eBasicTypeUnsignedLong;
    case builtin_type_impl::kind_t::unsigned_long_long_:
        return lldb::eBasicTypeUnsignedLongLong;
    case builtin_type_impl::kind_t::float_:
        return lldb::eBasicTypeFloat;
    case builtin_type_impl::kind_t::double_:
        return lldb::eBasicTypeDouble;
    case builtin_type_impl::kind_t::long_double_:
        return lldb::eBasicTypeLongDouble;
    case builtin_type_impl::kind_t::wchar_t_:
        return lldb::eBasicTypeWChar;
    case builtin_type_impl::kind_t::char16_t_:
        return lldb::eBasicTypeChar16;
    case builtin_type_impl::kind_t::char32_t_:
        return lldb::eBasicTypeChar32;
    case builtin_type_impl::kind_t::nullptr_t_:
        return lldb::eBasicTypeNullPtr;
    case builtin_type_impl::kind_t::complex_char_:
    case builtin_type_impl::kind_t::complex_short_:
    case builtin_type_impl::kind_t::complex_int_:
    case builtin_type_impl::kind_t::complex_long_:
    case builtin_type_impl::kind_t::complex_long_long_:
    case builtin_type_impl::kind_t::complex_unsigned_char_:
    case builtin_type_impl::kind_t::complex_unsigned_short_:
    case builtin_type_impl::kind_t::complex_unsigned_int_:
    case builtin_type_impl::kind_t::complex_unsigned_long_:
    case builtin_type_impl::kind_t::complex_unsigned_long_long_:
        throw dbgfmt::backend::format_error{"creating of integer complex values is not supported"};
    case builtin_type_impl::kind_t::complex_float_:
        return lldb::eBasicTypeFloatComplex;
    case builtin_type_impl::kind_t::complex_double_:
        return lldb::eBasicTypeDoubleComplex;
    case builtin_type_impl::kind_t::complex_long_double_:
        return lldb::eBasicTypeLongDoubleComplex;
    default:
        throw dbgfmt::backend::format_error{"Unknown builtin type kind"};
    }
}


type_impl * target_base::bt_type(dbgfmt::backend::builtin_type_impl::kind_t kind) {
    auto ts = lldb_targ().GetSP()->GetScratchTypeSystemForLanguage(lldb::eLanguageTypeC_plus_plus_14);
    auto comp_type = ts.get()->GetBasicTypeFromAST(get_lldb_basic_type(kind));
    lldb::SBType sb_type{comp_type};
    return get_or_create_type(sb_type);
}


const type_impl * target_base::dynamic_type_at_addr(uint64_t & addr, const dbgfmt::backend::type_impl * static_type) {
    auto core_static_type = dynamic_cast<const type_impl*>(static_type);
    assert(core_static_type && "invalid type passed in dynamic_type_at_addr");
    return core_static_type->dynamic_type_at_addr(addr);
}


dbgfmt::backend::debug_object_impl * target_base::obj_at_addr(uint64_t addr) {

    auto sbaddr = lldb_targ().ResolveLoadAddress(addr);

    // trying get function or global variable
    auto ctx = sbaddr.GetSymbolContext(lldb::eSymbolContextFunction |
                                       lldb::eSymbolContextVariable |
                                       lldb::eSymbolContextSymbol);

    if (!ctx.IsValid()) {
        // can't get symbol context for address
        return nullptr;
    }

    // sometimes lldb can't parse variable type so we have to check it's not null
    auto var = ctx.get()->variable;
    if (var && var->GetType() != nullptr) {
        // detecting variable start address by symbol address if it exists
        uint64_t var_address = addr;
        auto sym = ctx.GetSymbol();
        if (sym.IsValid()) {
            var_address = sym.GetStartAddress().GetLoadAddress(lldb_targ());
        }

        return get_or_create_global_var(var, var_address);
    }

    // trying get function
    auto func = ctx.GetFunction();
    if (func.IsValid()) {
        // looking for existing debug object for function
        auto & fobj = func_objects_[func];
        if (fobj) {
            return fobj.get();
        }

        // creating new debug object for function
        fobj = std::make_unique<function_debug_object_impl>(*this, func);

        return fobj.get();
    }

    return nullptr;
}


dbgfmt::backend::symbol_impl * target_base::sym_at_addr(uint64_t addr) {
    // looking for lldb symbol at address
    auto sbaddr = lldb_targ().ResolveLoadAddress(addr);
    auto sym = sbaddr.GetSymbol();
    if (!sym.IsValid()) {
        return nullptr;
    }

    // looking for existing symbol object
    auto & sobj = sym_objects_[sym];
    if (sobj) {
        return sobj.get();
    }

    // creating new symbol object
    sobj = std::make_unique<symbol_impl>(*this, sym);

    return sobj.get();
}


type_impl * target_base::get_or_create_ptr_type(const type_impl * type) {
    // looking for existing pointer
    auto it = ptr_types_.find(type);
    if (it != ptr_types_.end()) {
        return it->second;
    }

    // creating new pointer type
    auto lldb_ptr = type->lldb_type().GetPointerType();
    if (!lldb_ptr.IsValid()) {
        return nullptr;
    }

    auto new_type = get_or_create_type(lldb_ptr);
    auto ins_res = ptr_types_.emplace(type, new_type);
    assert(ins_res.second && "pointer type should not exist");
    return new_type;
}


type_impl * target_base::get_or_create_type(const lldb::SBType & tp) {
    auto tp2 = tp;

    auto & res = types_[tp2];
    if (res) {
        return res.get();
    }

    switch (tp2.GetTypeClass()) {
    case lldb::eTypeClassBuiltin:
    case lldb::eTypeClassComplexFloat:
    case lldb::eTypeClassComplexInteger:
        res = std::make_shared<builtin_type_impl>(*this, tp);
        break;
    case lldb::eTypeClassArray:
        res = std::make_shared<array_type_impl>(*this, tp);
        break;
    case lldb::eTypeClassVector:
        res = std::make_shared<vector_type_impl>(*this, tp);
        break;
    case lldb::eTypeClassPointer:
    case lldb::eTypeClassReference:
        res = std::make_shared<lldb_pointer_type_impl>(*this, tp);
        break;
    case lldb::eTypeClassMemberPointer:
        res = std::make_shared<mem_ptr_type_impl>(*this, tp);
        break;
    case lldb::eTypeClassClass:
    case lldb::eTypeClassStruct:
    case lldb::eTypeClassUnion:
        res = std::make_shared<record_type_impl>(*this, tp);
        break;
    case lldb::eTypeClassTypedef:
        res = std::make_shared<typedef_type_impl>(*this, tp);
        break;
    case lldb::eTypeClassEnumeration:
        res = std::make_shared<enum_type_impl>(*this, tp);
        break;
    case lldb::eTypeClassFunction:
        res = std::make_shared<function_type_impl>(*this, tp);
        break;
    default:
        throw dbgfmt::backend::format_error{"unknown lldb type"};
    }

    assert(res && "result type should not be null here");
    return res.get();
}


dbgfmt::data::endianness target_base::endianness() const {
    if (lldb_targ().GetByteOrder() == lldb::eByteOrderLittle) {
        return dbgfmt::data::endianness::little;
    } else {
        return dbgfmt::data::endianness::big;
    }
}


target_base::state_t target_base::state() const {
    lldb::SBTarget targ = targ_;
    lldb::SBProcess proc = targ.GetProcess();
    if (!proc.IsValid()) {
        // target not launched
        return state_t::loaded;
    }

    return lldb_state_to_state(proc.GetState());
}


int target_base::exit_code() const {
    assert(state() == state_t::loaded &&
           "exit_code can be called only in loaded state");

    lldb::SBTarget targ = targ_;
    lldb::SBProcess proc = targ.GetProcess();
    if (!proc.IsValid())
        return -1;

    return proc.GetExitStatus();
}


int target_base::signal_num() const {
    assert(state() == state_t::stopped &&
           "signal_num should be called only in stopped state");

    // searching for thread with signal stop reason

    lldb::SBProcess proc = lldb_targ().GetProcess();
    assert(proc.IsValid() && "Can't get LLDB process for target");

    for (uint32_t i = 0, e = proc.GetNumThreads(); i != e; ++i) {
        lldb::SBThread thrd = proc.GetThreadAtIndex(i);
        assert(thrd.IsValid() && "Can't get thread from LLDB process");

        if ((thrd.GetStopReason() == lldb::eStopReasonSignal ||
             thrd.GetStopReason() == lldb::eStopReasonException) &&
            thrd.GetStopReasonDataCount() > 0) {
            return static_cast<int>(thrd.GetStopReasonDataAtIndex(0));
        }
    }

    return 0;
}


breakpoint_num target_base::active_bp_num() const {
    assert(state() == state_t::stopped &&
           "signal_num should be called only in stopped state");

    lldb::SBProcess proc = lldb_targ().GetProcess();
    assert(proc.IsValid() && "Can't get LLDB process for target");

    for (uint32_t i = 0, e = proc.GetNumThreads(); i != e; ++i) {
        lldb::SBThread thrd = proc.GetThreadAtIndex(i);
        assert(thrd.IsValid() && "Can't get thread from LLDB process");

        if ((thrd.GetStopReason() == lldb::eStopReasonBreakpoint) &&
            thrd.GetStopReasonDataCount() > 0) {
            return static_cast<breakpoint_num>(thrd.GetStopReasonDataAtIndex(0));
        }
    }

    return 0;
}


breakpoint_num target_base::watchpoint_num() const {
    assert(state() == state_t::stopped &&
           "signal_num should be called only in stopped state");

    lldb::SBProcess proc = lldb_targ().GetProcess();
    assert(proc.IsValid() && "Can't get LLDB process for target");

    for (uint32_t i = 0, e = proc.GetNumThreads(); i != e; ++i) {
        lldb::SBThread thrd = proc.GetThreadAtIndex(i);
        assert(thrd.IsValid() && "Can't get thread from LLDB process");

        if ((thrd.GetStopReason() == lldb::eStopReasonWatchpoint) &&
            thrd.GetStopReasonDataCount() > 0) {
            return static_cast<breakpoint_num>(thrd.GetStopReasonDataAtIndex(0));
        }
    }

    return 0;
}


unsigned long target_base::launch(const std::filesystem::path & work_dir,
                                  const std::vector<std::string> & args,
                                  const std::map<std::string, std::string> & add_env) {

    CXXDBG_CORE_LOG_DEBUG << "launching";

    assert(state() == state_t::loaded && "Invalid target state");

    check_can_debug();

    // building vector of pointers to arguments
    std::vector<const char*> arg_ptrs;
    for (auto it = args.begin(), end = args.end(); it != end; ++it) {
        arg_ptrs.push_back(it->c_str());
    }

    arg_ptrs.push_back(nullptr);

    // building launch info
    lldb::SBLaunchInfo info(nullptr);
    info.SetWorkingDirectory(work_dir.string().c_str());
    info.SetArguments(&arg_ptrs[0], false);

    auto env = targ_.GetSP()->GetEnvironment();

    for (auto && e : add_env) {
        env[e.first] = e.second;
    }

    std::vector<std::unique_ptr<std::string>> envPairs;
    std::vector<const char*> envPtrs;
    envPairs.reserve(env.size());
    envPtrs.reserve(env.size() + 1);
    for (auto && entry : env) {
        std::string pair = (entry.first() + "=" + entry.second).str();
        envPairs.push_back(std::make_unique<std::string>(pair));
        envPtrs.push_back(envPairs.back()->c_str());
    }

    envPtrs.push_back(0);
    info.SetEnvironmentEntries(&envPtrs[0], false);

    // launching
    lldb::SBError err;
    lldb::SBProcess proc = targ_.Launch(info, err);
    if (!proc.IsValid() || err.Fail()) {
        std::ostringstream msg;
        auto err_cstr = err.GetCString();
        msg << "can't start process: " << (err_cstr ? err_cstr : "unknown error");
        CXXDBG_CORE_LOG_ERROR << "launch error: " << msg.str();
        throw std::runtime_error{msg.str()};
    }

    CXXDBG_CORE_LOG_DEBUG << "launch complete, process ID: " << proc.GetProcessID()
        << ", status after launch: " << proc.GetState();
    return static_cast<unsigned long>(proc.GetProcessID());
}


void target_base::attach(const std::variant<unsigned long, std::string> & targ) {

    assert(state() == state_t::loaded && "Invalid target state");

    check_can_debug();

    lldb::SBFileSpec old_exe = targ_.GetExecutable();

    // attaching to process
    lldb::SBListener list;
    lldb::SBError err;
    lldb::SBProcess proc;
    std::string targ_name;
    if (auto pid = std::get_if<unsigned long>(&targ)) {
        targ_name = std::to_string(*pid);
        proc = targ_.AttachToProcessWithID(list, static_cast<lldb::pid_t>(*pid), err);
    } else if (auto name = std::get_if<std::string>(&targ)) {
        targ_name = *name;
        proc = targ_.AttachToProcessWithName(list, name->c_str(), false, err);
    } else {
        assert(false && "unknown attach target");
    }

    if (err.Fail()) {
        std::ostringstream msg;
        msg << "Can't attach to process " << targ_name << ": " << err.GetCString();
        throw std::runtime_error(msg.str());
    }

    if (!proc.IsValid()) {
        std::ostringstream msg;
        auto err_cstr = err.GetCString();
        msg << "Can't attach to process " << targ_name << ": ";
        if (err_cstr) {
            msg << err_cstr;
        } else {
            msg << "unknown error";
        }

        throw std::runtime_error(msg.str());
    }

    // new lldb does not send notification about changing state after
    // attaching, sending it
    auto st = lldb_state_to_state(proc.GetState());
    CXXDBG_CORE_LOG_DEBUG << "changing state after attach to " << (int)st;
    state_changed_signal_(st);

    assert(proc.IsValid() && "AttachToProcessWithID returned bad process and no error");
}


void target_base::run() {

    assert(state() == state_t::stopped && "Invalid target state");

    // getting lldb process
    lldb::SBProcess proc = targ_.GetProcess();

    // checking lldb process is valid
    assert(proc.IsValid() && "lldb process is invalid");

    // checking process state
    assert(proc.GetState() == lldb::eStateStopped &&
           "lldb process state is invalid");

    // starting process
    lldb::SBError err = proc.Continue();
    if (err.Fail()) {
        std::ostringstream str;
        str << "Error running process: " << err.GetCString();
        throw std::runtime_error(str.str());
    }
}


void target_base::stop() {

    assert((state() == state_t::running || state() == state_t::stopped) &&
           "Invalid target state");

    // getting lldb process
    lldb::SBProcess proc = targ_.GetProcess();
    assert(proc.IsValid() && "lldb process is invalid");

    // checking process state
    assert((proc.GetState() == lldb::eStateStopped ||
            proc.GetState() == lldb::eStateRunning) &&
           "Invalid lldb process state");

    // stopping lldb process
    lldb::SBError err = proc.Stop();
    if (err.Fail()) {
        std::ostringstream str;
        str << "Error stopping process: " << err.GetCString();
        throw std::runtime_error(str.str());
    }
}


void target_base::terminate() {

    assert(state() == state_t::stopped && "Invalid target state");

    // getting lldb process
    lldb::SBProcess proc = targ_.GetProcess();
    assert(proc.IsValid() && "lldb process is invalid");

    // checking that lldb process state was not changed
    lldb::StateType st = proc.GetState();
    assert(st == lldb::eStateStopped && "Invalid lldb process satate");

    // killing lldb process
    lldb::SBError err = proc.Kill();
    if (err.Fail()) {
        std::ostringstream str;
        str << "Error terminating process: " << err.GetCString();
        throw std::runtime_error(str.str());
    }

    // // debugging log for catching error
    // if (state() != state_t::loaded) {
    //     std::cerr << "ERR: bad state after terminate: ";
    //     switch (state()) {
    //     case state_t::loaded:
    //         std::cerr << "loaded";
    //         break;
    //     case state_t::launching:
    //         std::cerr << "launching";
    //         break;
    //     case state_t::running:
    //         std::cerr << "running";
    //         break;
    //     case state_t::stopped:
    //         std::cerr << "stopped";
    //         break;
    //     default:
    //         std::cerr << "unknown";
    //         break;
    //     }

    //     std::cerr << "\n";
    // }

    // assert(state() == state_t::loaded && "Invalid target state");
}


void target_base::detach() {
    assert(state() == state_t::stopped && "Invalid target state");

    // getting lldb process
    lldb::SBProcess proc = targ_.GetProcess();
    assert(proc.IsValid() && "lldb process is invalid");

    // checking that lldb process state was not changed
    lldb::StateType st = proc.GetState();
    assert(st == lldb::eStateStopped && "Invalid lldb process satate");

    // detaching from process
    lldb::SBError err = proc.Detach();
    if (err.Fail()) {
        std::ostringstream str;
        str << "Error terminating process: " << err.GetCString();
        throw std::runtime_error(str.str());
    }

    assert(state() == state_t::loaded && "Invalid target state");
}


void target_base::step_into(unsigned long thread_id,
                       bool avoid_nodebug,
                       const std::string & target,
                       const std::string & avoid_regex,
                       const std::string & step_through_regex) {

    // getting lldb process
    lldb::SBProcess proc = targ_.GetProcess();
    assert(proc.IsValid() && "lldb process is invalid");

    // checking process state
    assert(proc.GetState() == lldb::eStateStopped &&
           "lldb process state is invalid");

    // getting thread with specified id
    lldb::SBThread thread = proc.GetThreadByID(thread_id);
    assert(thread.IsValid() && "Thread is not valid");

    // stepping into on thread
    lldb::XLazyBool lldbAvoidNodebug = avoid_nodebug ?
                                       lldb::eXLazyBoolYes :
                                       lldb::eXLazyBoolNo;
    lldb::SBError error;
    thread.StepInto(target.empty() ? nullptr : target.c_str(),
                    LLDB_INVALID_LINE_NUMBER,
                    error,
                    lldb::eOnlyDuringStepping,
                    lldbAvoidNodebug,
                    avoid_regex,
                    step_through_regex);
}


void target_base::step_over(unsigned long thread_id, const std::string & step_through_regex) {

    // getting lldb process
    lldb::SBProcess proc = targ_.GetProcess();
    assert(proc.IsValid() && "lldb process is invalid");

    // checking process state
    assert(proc.GetState() == lldb::eStateStopped &&
           "lldb process state is invalid");

    // getting thread with specified id
    lldb::SBThread thread = proc.GetThreadByID(thread_id);
    assert(thread.IsValid() && "Thread is not valid");

    // stepping into on thread
    thread.StepOver(lldb::eOnlyDuringStepping, step_through_regex);
}


void target_base::step_out(unsigned long thread_id,
                      bool avoid_nodebug,
                      const std::string & step_through_regex) {

    // getting lldb process
    lldb::SBProcess proc = targ_.GetProcess();
    assert(proc.IsValid() && "lldb process is invalid");

    // checking process state
    assert(proc.GetState() == lldb::eStateStopped &&
           "lldb process state is invalid");

    // getting thread with specified id
    lldb::SBThread thread = proc.GetThreadByID(thread_id);
    assert(thread.IsValid() && "Thread is not valid");

    // stepping into on thread
    lldb::SBError err;
    thread.StepOut(err, avoid_nodebug ? lldb::eXLazyBoolYes : lldb::eXLazyBoolNo,
                   step_through_regex);
}


void target_base::step_inst_into(unsigned long thread_id) {

    // getting lldb process
    lldb::SBProcess proc = targ_.GetProcess();
    assert(proc.IsValid() && "lldb process is invalid");

    // checking process state
    assert(proc.GetState() == lldb::eStateStopped &&
           "lldb process state is invalid");

    // getting thread with specified id
    lldb::SBThread thread = proc.GetThreadByID(thread_id);
    assert(thread.IsValid() && "Thread is not valid");

    // stepping into on thread
    thread.StepInstruction(false);
}


void target_base::step_inst_over(unsigned long thread_id) {

    // getting lldb process
    lldb::SBProcess proc = targ_.GetProcess();
    assert(proc.IsValid() && "lldb process is invalid");

    // checking process state
    assert(proc.GetState() == lldb::eStateStopped &&
           "lldb process state is invalid");

    // getting thread with specified id
    lldb::SBThread thread = proc.GetThreadByID(thread_id);
    assert(thread.IsValid() && "Thread is not valid");

    // stepping into on thread
    thread.StepInstruction(true);
}


void target_base::step_until(unsigned long thread_id, unsigned int line) {

    // getting lldb process
    lldb::SBProcess proc = targ_.GetProcess();
    assert(proc.IsValid() && "lldb process is invalid");

    // checking process state
    assert(proc.GetState() == lldb::eStateStopped &&
           "lldb process state is invalid");

    // getting thread with specified id
    lldb::SBThread thread = proc.GetThreadByID(thread_id);
    assert(thread.IsValid() && "Thread is not valid");

    // stepping until on thread
    lldb::SBFrame frame;
    lldb::SBFileSpec file;
    lldb::SBError err = thread.StepOverUntil(frame, file, line);
    if (err.Fail()) {
        std::ostringstream msg;
        msg << "Can't step until line " << line << ": " << err.GetCString();
        throw std::runtime_error(msg.str());
    }
}


target_base::connection target_base::connect_state_changed(const state_changed_handler & handl) {
    return state_changed_signal_.connect(handl);
}


unsigned long target_base::pid() const {
    assert((state() == state_t::running || state() == state_t::stopped) &&
           "Invalid target state");

    lldb::SBProcess proc = lldb_targ().GetProcess();
    assert(proc.IsValid() && "Can't get lldb process");

    return proc.GetProcessID();
}


std::size_t target_base::threads_size() const {
    lldb::SBProcess proc = lldb_targ().GetProcess();
    assert(proc.IsValid() && "Invalid lldb process");

    return static_cast<std::size_t>(proc.GetNumThreads());
}


std::size_t target_base::current_thread_index() {
    lldb::SBProcess proc = lldb_targ().GetProcess();
    assert(proc.IsValid() && "Invalid lldb process");

    lldb::SBThread cur_thread = proc.GetSelectedThread();
    for (uint32_t i = 0, e = proc.GetNumThreads(); i < e; ++i) {
        lldb::SBThread thrd = proc.GetThreadAtIndex(i);
        if (thrd.GetThreadID() == cur_thread.GetThreadID()) {
            return static_cast<std::size_t>(i);
        }
    }

    assert(false && "Can't find current thread");
    return 0;
}


void target_base::set_current_thread_index(std::size_t index) {
    lldb::SBProcess proc = lldb_targ().GetProcess();
    assert(proc.IsValid() && "Invalid lldb process");

    lldb::SBThread thrd = proc.GetThreadAtIndex(index);
    assert(thrd.IsValid() && "Can't find thread with specified index");

    proc.SetSelectedThread(thrd);
}


void target_base::set_current_stack_frame_index(std::size_t index) {
    lldb::SBProcess proc = lldb_targ().GetProcess();
    assert(proc.IsValid() && "Invalid lldb process");

    lldb::SBThread thrd = proc.GetSelectedThread();
    assert(thrd.IsValid() && "Invalid lldb selected thread");

    thrd.SetSelectedFrame(index);
}


global_variable * target_base::get_or_create_global_var(lldb_private::Variable * var,
                                                   uint64_t addr) {
    // looking for existing global_variable object in map
    auto & gvar = glob_vars_[var];
    if (gvar) {
        return gvar.get();
    }

    // creating new global_variable object if does not eixst
    gvar = std::make_unique<global_variable>(*this, var, addr);
    return gvar.get();
}


variable * target_base::get_or_create_var(const lldb::VariableSP & v) {
    // looking for existing variable object in map
    auto & var = vars_[v];
    if (var) {
        return var.get();
    }

    // creating new global_variable object if does not eixst
    var = std::make_unique<variable>(v);
    return var.get();
}


breakpoint * target_base::get_breakpoint_by_id(const breakpoint::id_t & id) {
    // searching breakpoint in list of breakpoints
    auto it = breakpoints_.find(id);
    if (it == breakpoints_.end()) {
        return nullptr;
    }

    return it->second.get();
}


const breakpoint * target_base::get_breakpoint_by_id(const breakpoint::id_t & id) const {
    // searching breakpoint in list of breakpoints
    auto it = breakpoints_.find(id);
    if (it == breakpoints_.end()) {
        return nullptr;
    }

    return it->second.get();
}


code_breakpoint * target_base::add_breakpoint(const source_position & pos) {
    // creating new lldb breakpoint
    lldb::SBBreakpoint bp = lldb_targ().BreakpointCreateByLocation(pos.path().string().c_str(), pos.line());
    if (!bp.IsValid()) {
        throw std::runtime_error("Can't add lldb breakpoint");
    }

    // creating breakpoint object
    auto b = std::make_shared<source_position_breakpoint>(*this, bp, pos);
    breakpoints_.insert(std::make_pair(b->id(), b));
    return b.get();
}


code_breakpoint * target_base::add_breakpoint(const std::string & func_name) {
    // creating new lldb breakpoint
    lldb::SBBreakpoint bp = lldb_targ().BreakpointCreateByName(func_name.c_str());
    if (!bp.IsValid()) {
        throw std::runtime_error("Can't add lldb breakpoint");
    }

    // creating breakpoint object
    auto b = std::make_shared<function_breakpoint>(*this, bp, func_name);
    breakpoints_.emplace(b->id(), b);
    return b.get();
}


code_breakpoint * target_base::add_breakpoint(std::uint64_t addr) {
    // creating new lldb breakpoint
    lldb::SBBreakpoint bp = lldb_targ().BreakpointCreateByAddress(addr);
    if (!bp.IsValid()) {
        throw std::runtime_error("Can't add lddb breakpoint");
    }

    // creating breakpoint object
    auto b = std::make_shared<address_breakpoint>(*this, bp, addr);
    breakpoints_.emplace(b->id(), b);
    return b.get();
}


void target_base::remove_breakpoint(breakpoint * bp) {
    // removing lldb breakpoint
    bp->remove();

    // removing breakpoint from list
    breakpoints_.erase(bp->id());
}


// Checks watchpoint and throws exception is it's not valid
static void check_watchpoint(lldb::SBWatchpoint wp, lldb::SBError err) {
    if (wp.IsValid()) {
        return;
    }

    std::ostringstream msg;
    auto err_str = err.GetCString();
    if (err_str) {
        msg << err_str;
    } else {
        // trying get error from watchpoint
        auto wp_err = wp.GetError();
        err_str = wp_err.GetCString();
        if (err_str) {
            msg << err_str;
        } else {
            msg << "unknown error while trying set watchpoint";
        }
    }

    throw std::runtime_error{msg.str()};
}


watchpoint * target_base::add_var_watchpoint(const std::string & var_name,
                                             bool read,
                                             bool write,
                                             size_t size) {
    // TODO implement setting size in LLDB

    // looking for variable
    auto var = find_var(var_name);
    if (!var.IsValid()) {
        std::ostringstream msg;
        msg << "can't find variable '" << var_name << "'";
        throw std::runtime_error{msg.str()};
    }

    // trying set watchpoint
    lldb::SBError err;
    auto wp = var.Watch(true, read, write, err);

    // checking for error
    check_watchpoint(wp, err);

    // creating watchpoint object
    auto wp_obj = std::make_shared<watchpoint>(*this, wp);
    breakpoints_.emplace(wp_obj->id(), wp_obj);
    return wp_obj.get();
}


watchpoint *target_base::add_expr_watchpoint(const std::string & expr,
                                        bool read,
                                        bool write,
                                        size_t size) {
    // TODO implement setting size in LLDB

    // evaluating expression
    lldb::SBExpressionOptions opts;
    auto res = lldb_targ().EvaluateExpression(expr.c_str(), opts);
    auto res_err = res.GetError();

    // checking for error
    if (!res.IsValid() || (res_err.IsValid() && res_err.Fail())) {
        std::ostringstream msg;
        msg << "can't evaluate expression: ";

        auto err_str = res_err.GetCString();
        if (err_str) {
            msg << err_str;
        } else {
            msg << "unknown error";
        }

        throw std::runtime_error{msg.str()};
    }

    // trying set watchpoint
    lldb::SBError err;
    auto wp = res.WatchPointee(true, read, write, err);

    // checking for error
    check_watchpoint(wp, err);

    // creating watchpoint object
    auto wp_obj = std::make_shared<watchpoint>(*this, wp);
    breakpoints_.emplace(wp_obj->id(), wp_obj);
    return wp_obj.get();
}


void target_base::send_stdin(const std::string & data) {
    assert((state() == state_t::running || state() == state_t::stopped) &&
           "can't send stdin to stopped target");

    auto proc = targ_.GetProcess();
    assert(proc.IsValid() && "invalid lldb process");

    proc.PutSTDIN(data.c_str(), data.size());
}


std::vector<instruction> target_base::disassemble(uint64_t addr, int64_t inst_offset, uint64_t inst_count) {
    lldb_disassembbler lldb_dis{*this};
    memory_disassembler mem_dis{lldb_dis};
    return mem_dis.disassemble(addr, inst_offset, inst_count);
}


std::vector<char> target_base::read_memory(uint64_t addr, uint64_t size) {
    // reading memory
    std::vector<char> data;
    data.resize(size);
    auto sbaddr = lldb_targ().ResolveLoadAddress(addr);
    lldb::SBError err;
    auto nread = lldb_targ().ReadMemory(sbaddr, data.data(), data.size(), err);

    // truncating result vector if not all data was read
    if (nread != size) {
        data.resize(nread);
    }

    return data;
}


std::vector<std::string> target_base::call_targets(const code_position & pos) const {
    auto addr = pos.lldb_addr();
    if (!addr.IsValid()) {
        return {};
    }

    auto line = addr.GetLineEntry();
    if (!line.IsValid()) {
        return {};
    }

    // getting address range

    auto startAddr = line.GetStartAddress();
    auto endAddr = line.GetEndAddress();
    if (!startAddr.IsValid() || !endAddr.IsValid()) {
        return {};
    }

    auto sz = endAddr.GetLoadAddress(lldb_targ()) - startAddr.GetLoadAddress(lldb_targ());
    if (sz >= 1024 * 1024) {
        // too big address range
        return {};
    }


    // disassembling address range
    auto insts = lldb_targ().ReadInstructions(startAddr, sz);

    std::vector<std::string> res;

    // scanning instruction range for call targets
    for (size_t i = 0, e = insts.GetSize(); i < e; ++i) {
        auto inst = insts.GetInstructionAtIndex(i);

        // checking for end for address range because bytes count
        // is greater than instructions count
        if (inst.GetAddress().GetLoadAddress(lldb_targ()) > endAddr.GetLoadAddress(lldb_targ())) {
            break;
        }

        auto name_cstr = inst.GetMnemonic(lldb_targ());
        auto args_cstr = inst.GetOperands(lldb_targ());
        if (name_cstr == nullptr || args_cstr == nullptr) {
            continue;
        }

        std::string name = name_cstr;
        std::string args = args_cstr;

        // checking if instruction is call instruction
        if (name.substr(0, 4) != "call" &&      // intel call
            name.substr(0, 2) != "bl") {        // arm call
            continue;
        }

        // checking if argument starts from '0x'
        if (args.substr(0, 2) != "0x") {
            continue;
        }

        // trying parse operand as address
        std::istringstream args_str{args.substr(2)};
        uint64_t addr_imm = 0;
        args_str >> std::hex >> addr_imm;
        if (args_str.fail() || !args_str.eof()) {
            // can't parse address
            continue;
        }

        // trying extract function name at address
        auto t = lldb_targ();
        auto fname = func_name_at_address(cm(), lldb::SBAddress{addr_imm, t});
        if (fname.empty()) {
            // can't extract function name
            continue;
        }

        res.push_back(fname);
    }

    return res;
}


const code_model & target_base::cm() const {
    return dbg_.cm();
}


lldb::SBThread target_base::lldb_thread_at(std::size_t i) {
    lldb::SBProcess proc = targ_.GetProcess();
    assert(proc.IsValid() && "Invalid lldb process");
    return proc.GetThreadAtIndex(i);
}


target_base::target_base(debugger_base & dbg, const lldb::SBTarget & lt):
dbg_(dbg), targ_(lt), sym_objects_{compare_lldb_sym{lt}}, mem_{lt} {

    // listening for state changed and modules changed LLDB events
    lldb_event_con_ = dbg_.connect_lldb_event([this](lldb::SBEvent ev) {
        if (lldb::SBProcess::EventIsProcessEvent(ev)) {
            if (ev.GetType() == lldb::SBProcess::eBroadcastBitSTDOUT) {
                CXXDBG_LOG_SCAT_DEBUG(core, target) << "received lldb process stdout event";
                handle_stdout();
                return;
            }

            if (ev.GetType() == lldb::SBProcess::eBroadcastBitSTDERR) {
                CXXDBG_LOG_SCAT_DEBUG(core, target) << "received lldb process stderr event";
                handle_stderr();
                return;
            }

            if (ev.GetType() != lldb::SBProcess::eBroadcastBitStateChanged) {
                CXXDBG_LOG_SCAT_DEBUG(core, target) << "received unknown lldb process event, ignoring";
                return;
            }

            CXXDBG_LOG_SCAT_DEBUG(core, target) << "received lldb process state event";

            lldb::SBProcess proc = lldb::SBProcess::GetProcessFromEvent(ev);
            assert(proc.IsValid() && "Invalid lldb process");

            // checking process id
            if (proc.GetProcessID() != targ_.GetProcess().GetProcessID()) {
                CXXDBG_LOG_SCAT_DEBUG(core, target) << "process state event for other process, ignoring";
                return;
            }

            // checking restarted flag
            lldb::StateType st = lldb::SBProcess::GetStateFromEvent(ev);
            if ((st == lldb::eStateStopped || st == lldb::eStateCrashed || st == lldb::eStateSuspended) &&
                lldb::SBProcess::GetRestartedFromEvent(ev)) {
                // process was stopped and restarted
                // (breakpoint with ignore, received SIGCHLD signal, or something)
                CXXDBG_LOG_SCAT_DEBUG(core, target) << "process was stopped and restarted, ignoring event";
                return;
            }

            auto cxxdbg_st = lldb_state_to_state(st);

            // clearing all variables in map if there is no process
            if (cxxdbg_st != state_t::running && cxxdbg_st != state_t::stopped) {
                glob_vars_.clear();
            }

            CXXDBG_LOG_SCAT_DEBUG(core, target) << "notifying state change to " << state_name(cxxdbg_st);
            state_changed_signal_(cxxdbg_st);

        } else if (ev.GetBroadcaster() == targ_.GetBroadcaster()) {
            if (ev.GetType() != lldb::SBTarget::eBroadcastBitModulesLoaded &&
                ev.GetType() != lldb::SBTarget::eBroadcastBitModulesUnloaded) {
                CXXDBG_LOG_SCAT_DEBUG(core, target) << "received unknown lldb event, ignoring";
                return;
            }

            CXXDBG_LOG_SCAT_DEBUG(core, target) << "received modules lldb event";

            std::vector<module> modules;
            auto num_mods = lldb::SBTarget::GetNumModulesFromEvent(ev);
            modules.reserve(num_mods);
            for (uint32_t i = 0; i < num_mods; ++i) {
                modules.push_back(module{lldb::SBTarget::GetModuleAtIndexFromEvent(i, ev)});
            }

            if (ev.GetType() == lldb::SBTarget::eBroadcastBitModulesLoaded) {
                modules_loaded(modules);
            } else if (ev.GetType() == lldb::SBTarget::eBroadcastBitModulesUnloaded) {
                modules_unloaded(modules);
            } else {
                assert(false && "invalid modules event");
            }
        }
    });

    reindex();
}


void target_base::check_can_debug() const {
    // we can't debug 64-bit target in 32-bit debugger
    // TODO: check for other arches
#if !defined(__LP64__) && !defined(_WIN64)
    if (lldb_targ().GetAddressByteSize() == 8) {
        throw std::runtime_error{"Can't debug 64-bit executables with"
                                 " 32-bit debugger. Please use 32-bit debugger"
                                 " instead."};
    }
#endif
}


void target_base::handle_stdout() {
    while (true) {
        char buf[1024];
        auto sz = targ_.GetProcess().GetSTDOUT(buf, sizeof(buf));

        if (sz == 0) {
            break;
        }

        stdout_received()(std::string{buf, sz});
    }
}


void target_base::handle_stderr() {
    while (true) {
        char buf[1024];
        auto sz = targ_.GetProcess().GetSTDOUT(buf, sizeof(buf));

        if (sz == 0) {
            break;
        }

        stderr_received()(std::string{buf, sz});
    }
}


lldb::SBValue target_base::find_var(const std::string & name) {
    // looking for variable in current frame
    if (state() == state_t::stopped) {
        auto frame = selected_frame_base();
        auto vars = frame.read_locals();
        auto it = std::find_if(vars.begin(), vars.end(), [&name](const auto & v) {
            return v->name() == name;
        });

        if (it != vars.end()) {
            return frame.read_var_lldb_value(*it);
        }
    }

    // looking for global variable
    return lldb_targ().FindFirstGlobalVariable(name.c_str());
}


target_base::const_module_iterator::const_module_iterator(const target_base & targ, std::size_t i):
iterator_adaptor_(i),
targ_(targ) {
}


target_base::const_module_iterator::reference target_base::const_module_iterator::dereference() const {
    return targ_.module_at(this->base());
}


}
