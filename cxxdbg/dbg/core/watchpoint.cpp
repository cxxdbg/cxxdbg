// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file watchpoint.cpp
/// Contains implementation of the watchpoint class.

#include "watchpoint.hpp"
#include "target_base.hpp"


namespace cxxdbg::dbg::core {


/// Explicit instantiation of the breakpoint_base class for watchpoints
template class breakpoint_base_impl<lldb::SBWatchpoint, breakpoint_id::type_t::watch>;


watchpoint::watchpoint(target_base & targ, const lldb::SBWatchpoint wp):
breakpoint_base_impl<lldb::SBWatchpoint, breakpoint_id::type_t::watch>{wp},
targ_{targ} {
}


std::string watchpoint::old_value() const {
    return lldb_bp().GetOldValue();
}


std::string watchpoint::new_value() const {
    return lldb_bp().GetNewValue();
}


uint64_t watchpoint::watch_addr() const {
    return lldb_bp().GetWatchAddress();
}


void watchpoint::remove() {
    targ_.lldb_targ().DeleteWatchpoint(lldb_bp().GetID());
}


watchpoint::watchpoint(const watchpoint & w) = default;
watchpoint::watchpoint(watchpoint && w) = default;
watchpoint::~watchpoint() = default;


}
