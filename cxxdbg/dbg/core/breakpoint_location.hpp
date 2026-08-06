// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file breakpoint_location.hpp
/// Contains definition of breakpoint_location class.

#pragma once

#include "code_position.hpp"
#include <lldb/API/SBBreakpointLocation.h>


namespace cxxdbg::dbg::core {


/// \class breakpoint_location
/// Represents breakpoint location
class breakpoint_location {
public:
    /// Type of location id
    typedef lldb::break_id_t id_type;

    /// Constructor, makes breakpoint location with specified pointer to
    /// lldb breakpoint location
    breakpoint_location(target_base & t, const lldb::SBBreakpointLocation & l);

    /// Destructor, destroys object
    ~breakpoint_location();

    /// Returns location id
    id_type id() const;

    /// Returns position in code for breakpoint location
    code_position pos() const;

private:
    target_base & targ_;                    ///< Reference to target
    lldb::SBBreakpointLocation loc_;        ///< Pointer to breakpoint location
};


}
