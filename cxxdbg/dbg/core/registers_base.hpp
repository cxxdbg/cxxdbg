// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file registers.hpp
/// Contains definition of base classes for classes used to reading register.s

#pragma once

#include "dbgfmt/data/storage.hpp"
#include <lldb/API/SBValue.h>
#include <lldb/API/SBValueList.h>


namespace cxxdbg::dbg::core {

class target_base;
class type_impl;


/// Base class for registers_group. Contains functions dependent on LLDB
class registers_group_base {
public:
    /// Constructs register group with specified reference to target and LLDB value
    /// containing registers
    registers_group_base(target_base & targ, const lldb::SBValue & val);

    /// Copy constructor
    registers_group_base(const registers_group_base & grp);

    /// Destroys object
    ~registers_group_base();

    /// Returns name of group
    std::string name() const;

    /// Returns number of registers in list
    size_t registers_size() const;

    /// Returns name of register at specifeid index
    std::string register_name_at(size_t idx) const;

protected:
    /// Returns type of regsiter value at specified index
    const type_impl * register_value_type_at(size_t idx) const;

    /// Returns value storage for register value at specified index
    std::shared_ptr<dbgfmt::data::storage> register_value_storage_at(size_t idx) const;

    /// Returns reference to target
    target_base & targ() const { return targ_; }

private:
    lldb::SBValue val() const;

    target_base & targ_;            ///< Reference to target
    lldb::SBValue val_;             ///< LLDB value for registers group
};


/// Base class for registers list. Contains functions dependent from LLDB.
class registers_list_base {
public:
    /// Constructs registers list with specified reference to target and list of lldb values
    registers_list_base(target_base & targ, const lldb::SBValueList & vals);

    /// Copy constructor
    registers_list_base(const registers_list_base & rlb);

    /// Destroys registers list
    ~registers_list_base();

    /// Returns number of groups in list
    size_t groups_size() const;

protected:
    /// Returns group at specified index
    registers_group_base group_base_at(size_t idx) const;

private:
    lldb::SBValueList values() const;

    target_base & targ_;            ///< Reference to target
    lldb::SBValueList values_;      ///< Value list containing registers
};



}
