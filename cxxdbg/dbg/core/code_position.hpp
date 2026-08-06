// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file code_position.hpp
/// Contains implementation of code_position class.

#pragma once

#include "source_position.hpp"
#include <lldb/API/SBAddress.h>


namespace cxxdbg::dbg::core {


class module;
class target_base;


/// \class code_position
/// Represents position in code
class code_position {
    friend class target_base;

public:
    /// Default constructor, makes invalid code position for specified target
    code_position(target_base & t);

    /// Constructor, makes code position with specified LLDB address for
    /// specified target
    code_position(target_base & t, const lldb::SBAddress & addr);

    /// Copy constructor
    code_position(const code_position & pos);

    /// Destructor, destroys object
    ~code_position();

    /// Returns load address of code position for specified target, or
    /// -1 if load address is not available
    std::uint64_t load_addr() const;

    /// Returns file address of code position
    std::uint64_t file_addr() const;

    /// Finds source position for position in code if available
    source_position get_src_pos() const;

    /// Extracts function name from code position
    std::string get_func_name(bool get_params = false) const;

    /// Extracts module from code position
    module get_module() const;

    /// Offsets code position by specified amount
    code_position offset(long val) const;

    /// Assignment operator, copies code position
    code_position & operator=(const code_position & pos);

    /// Returns true if code position is valid
    operator bool() const;

private:
    /// Returns lldb address
    lldb::SBAddress lldb_addr() const;

    target_base & targ_;        ///< Reference to target
    lldb::SBAddress addr_;      ///< LLDB address
};


}
