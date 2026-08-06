// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file breakpoint_info.cpp
/// Contains implementation of breakpoint_info class.

#include "breakpoint_info.hpp"


namespace cxxdbg::dbg {


void code_breakpoint_info::add_location(const breakpoint_location_info & loc) {
    locations_.push_back(loc);
}


}
