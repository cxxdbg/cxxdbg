// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file breakpoint_location.cpp
/// Contains implementation of breakpoint_location class.

#include "breakpoint_location.hpp"


namespace cxxdbg::dbg::core {


breakpoint_location::breakpoint_location(target_base & t, const lldb::SBBreakpointLocation & l):
targ_{t}, loc_(l) {
}


breakpoint_location::~breakpoint_location() {
}


breakpoint_location::id_type breakpoint_location::id() const {
    lldb::SBBreakpointLocation l = loc_;
    return l.GetID();
}


code_position breakpoint_location::pos() const {
    lldb::SBBreakpointLocation l = loc_;
    return code_position(targ_, l.GetAddress());
}


}
