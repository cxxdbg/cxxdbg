// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file address_breakpoint.hpp
/// Contains definition of the address_breakpoint class.

#pragma once

#include "code_breakpoint.hpp"


namespace cxxdbg::dbg::core {


/// Represents breakpoint at address
class address_breakpoint: public code_breakpoint {
public:
    /// Constructs address breakpoint object with specified reference to target,
    /// LLDB breakpoint, and address
    address_breakpoint(target_base & targ, const lldb::SBBreakpoint & bp, uint64_t a):
        code_breakpoint(targ, bp), addr_{a} {}

    /// Returns address for breakpoint
    uint64_t address() const { return addr_; }

private:
    uint64_t addr_;
};


}
