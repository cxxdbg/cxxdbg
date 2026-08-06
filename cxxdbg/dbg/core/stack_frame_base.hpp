// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file stack_frame_base.hpp
/// Contains definition of stack_frame_base class.

#pragma once

#include "code_position.hpp"
#include "registers_base.hpp"
#include "dbgfmt/data/storage.hpp"
#include <lldb/lldb-forward.h>
#include <lldb/API/SBValue.h>


namespace dbgfmt {
    class value_impl;
}


namespace cxxdbg::dbg::core {

class thread_base;
class type_impl;
class variable;


/// \class call_parameter
/// Represents call parameter in stack frame
/// TODO: replace with local variables
class call_parameter {
public:
    /// Constructor, makes parameter with specified pointer to LLDB value
    call_parameter(const lldb::SBValue & v);

    /// Copy constructor
    call_parameter(const call_parameter & par);

    /// Destructor, destroys object
    ~call_parameter();

    /// Returns parameter name
    std::string name() const;

    /// Returns parameter type name
    std::string type() const;

    /// Returns parameter value
    std::string value() const;

private:
    /// Returns pointer to LLDB value
    lldb::SBValue val() const;

    lldb::SBValue val_;         ///< LLDB value
};


/// \class stack_frame_base
/// Represents single stack frame in target being debugged. Base class for the stack_frame class,
/// contains code dependent from LLDB
class stack_frame_base {
public:
    /// Constructs stack frame from thread adn index of stack frame in thread
    stack_frame_base(const thread_base & thrd, size_t idx);

    /// Destructor, destroys object
    ~stack_frame_base();

    /// Reads code position info from frame
    code_position read_pos() const;

    /// Returns canonical frame address (CFA, stack pointer for frame)
    uint64_t get_cfa() const;

    /// Returns symbol context ID for frame pointer. Symbol context ID
    /// is used to distinct inlined functions in single stack frame
    uint64_t get_sc_id() const;

    /// Reads vector of call parameters
    std::vector<call_parameter> read_params() const;

    /// Reads vector of parameter variables in stack frame
    std::vector<const variable*> read_params_vars() const;

    /// Reads vector of local variables in stack frame
    std::vector<const variable*> read_locals(bool params = true) const;

    /// Reads LLDB value of frame variable
    lldb::SBValue read_var_lldb_value(const variable * var);

    /// Reads variable type
    type_impl * read_var_type(const variable * var);

    /// Tries get address of variable in target memory. Returns UINT64_MAX if
    /// value does not have address.
    uint64_t read_var_addr(const variable * var);

    /// Returns true if stack frame is valid
    bool is_valid() const;

protected:
    /// Creates and returns temporary storage for variable data
    std::shared_ptr<dbgfmt::data::storage> get_var_lldb_value_storage(const variable * var);

    /// Evaluates expression in stack frame. Returns pair of shared pointer to lldb value
    /// storage and type for evaluation result
    std::tuple<std::shared_ptr<dbgfmt::data::storage>, type_impl*> do_eval_expr(const std::string & expr) const;

    /// Reads registers list
    registers_list_base read_registers_base() const;

private:
    /// Returns pointer to lldb stack frame
    lldb::StackFrameSP frame() const;

    target_base & targ_;            ///< Reference to target for stack frame
    lldb::StackFrameSP frame_;      ///< Pointer to LLDB stack frame
};


}
