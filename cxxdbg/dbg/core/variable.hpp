// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file variable.hpp
/// Contains definition of variable class.

#pragma once

#include "source_position.hpp"
#include <lldb/lldb-forward.h>
#include <map>
#include <memory>


namespace dbgfmt {
    class value_impl;
}


namespace cxxdbg::dbg::core {


/// \class variable
/// Represents a variable in debugger
class variable: public std::enable_shared_from_this<variable> {
    friend class target_base;

public:
    /// Constructs variable with specified reference to target and pointer
    /// to LLDB variable object
    variable(lldb::VariableSP v);

    /// Destructor, destroys object and removes variable from map
    ~variable();

    /// Returns variable name
    std::string name() const;

    /// Returns source position of definition of variable
    source_position def_pos() const;

    /// Returns pointer to lldb variable
    lldb::VariableSP lldb_var() const;

    /// Returns variable ID
    unsigned long id() const;

private:
    /// Removed copy constructor
    variable(const variable &) = delete;

    /// Removed assignment operator
    variable & operator=(const variable &) = delete;


    lldb::VariableSP var_;      ///< Pointer to variable object
};


}
