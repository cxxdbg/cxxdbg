// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file source_position_breakpoint.hpp
/// Contains definition of the source_position_breakpoint class.

#pragma once

#include "code_breakpoint.hpp"


namespace cxxdbg::dbg::core {


/// Represents breakpoint at source position
class source_position_breakpoint: public code_breakpoint {
public:
    /// Constructs source position breakpoint object with specified reference to target,
    /// LLDB breakpoint, and source position
    source_position_breakpoint(target_base & targ, const lldb::SBBreakpoint & bp, source_position pos):
        code_breakpoint(targ, bp), pos_{std::move(pos)} {}

    /// Returns source position for breakpoint
    auto & pos() const { return pos_; }

private:
    source_position pos_;
};


}
