// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file registers.hpp
/// Contains definition of classes for reading registers,

#pragma once

#include "registers_base.hpp"
#include "dbgfmt/fmt_result.hpp"
#include "dbgfmt/backend/value_impl.hpp"
#include <lldb/API/SBValue.h>
#include <lldb/API/SBValueList.h>


namespace cxxdbg::dbg::core {

class target;


/// Group of registerss
class registers_group: public registers_group_base {
public:
    /// Constructs register group with specified registers_group_base instance
    registers_group(const registers_group_base & rgb):
        registers_group_base{rgb} {}

    /// Copy constructor
    registers_group(const registers_group &) = default;

    /// Destroys group
    ~registers_group() = default;

    /// Returns value of register at specified index
    std::shared_ptr<dbgfmt::backend::value_impl> register_value_at(size_t idx) const;
};


/// List of registers
class registers_list: public registers_list_base {
public:
    /// Constructs register list with specified reference to target and list of lldb values
    registers_list(target & targ, const lldb::SBValueList & vals);

    /// Constructs register list from registers_list_base instance
    registers_list(target & targ, const registers_list_base & rlb):
        registers_list_base{rlb}, targ_{targ} {}

    /// Destroys register list
    ~registers_list() = default;

    /// Returns register group at specified index
    registers_group group_at(size_t idx) const;

    /// Returns register group as formatting result
    dbgfmt::named_fmt_result group_fmt_result_at(size_t idx) const;

private:
    target & targ_;         ///< Reference to target
};


}
