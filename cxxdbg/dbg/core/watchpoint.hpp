// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file watchpoint.hpp
/// Contains definition of the watchpoint class

#pragma once

#include "breakpoint_base_impl.hpp"
#include <lldb/API/SBWatchpoint.h>


namespace cxxdbg::dbg::core {


class target_base;


/// Extern declaration of explicit instantiation of the breakpoint_base class.
/// We need this because we can't refer to lldb functions directly
/// outside of cxxdbgcore library
extern template class breakpoint_base_impl<lldb::SBWatchpoint, breakpoint_id::type_t::watch>;


/// Represents watchpoint in debugger
class watchpoint: public breakpoint_base_impl<lldb::SBWatchpoint, breakpoint_id::type_t::watch> {
public:
    /// Constructs watchpoint with specified reference to target
    /// and lldb watchpoint
    explicit watchpoint(target_base & targ, const lldb::SBWatchpoint wp);

    /// Copy constructor
    watchpoint(const watchpoint & w);

    /// Move constructor
    watchpoint(watchpoint && w);

    /// Destructor
    ~watchpoint();

    /// Returns formatted old value for watchpoint
    std::string old_value() const;

    /// Returns formatted new value for watchpoint
    std::string new_value() const;

    /// Returns watch address
    uint64_t watch_addr() const;

private:
    /// Removes breakpoint from target
    virtual void remove() override;

    target_base & targ_;                ///< Reference to target
};


using watchpoint_sp = std::shared_ptr<watchpoint>;


}
