// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file function_breakpoint.hpp
/// Contains definition of the function_breakpoint class.

#pragma once

#include "code_breakpoint.hpp"


namespace cxxdbg::dbg::core {


/// Represents breakpoint at function
class function_breakpoint: public code_breakpoint {
public:
    /// Constructs function breakpoint object with specified reference to target,
    /// LLDB breakpoint, and function name
    function_breakpoint(target_base & targ, const lldb::SBBreakpoint & bp, std::string f_name):
        code_breakpoint(targ, bp), func_name_{std::move(f_name)} {}

    /// Returns function name for breakpoint
    auto & func_name() const { return func_name_; }

private:
    std::string func_name_;
};


}
