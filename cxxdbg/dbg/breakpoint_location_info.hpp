// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file breakpoint_location_info.hpp
/// Contains definition of breakpoint_location_info class.

#pragma once

#include "code_position_info.hpp"


namespace cxxdbg::dbg {


/// \class breakpoint_location_info
/// Contains information about breakpoint location
class breakpoint_location_info {
public:
    /// Type of location ID
    typedef unsigned long id_type;

    /// Constructor, makes breakpoint location with specified
    /// location ID and source position
    breakpoint_location_info(id_type i, const code_position_info & p);

    /// Returns location ID
    id_type id() const;

    /// Returns code position for breakpoint location
    const code_position_info & pos() const;

private:
    id_type id_;                    ///< Location ID
    code_position_info pos_;        ///< Source position for breakpoint location
};


}


