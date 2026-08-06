// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file code_breakpoint.cpp
/// Contains implementation of the code_breakpoint class.

#include "code_breakpoint.hpp"
#include "target_base.hpp"


namespace cxxdbg::dbg::core {


/// Explicit instantiation of the breakpoint_base class
template class breakpoint_base_impl<lldb::SBBreakpoint, breakpoint_id::type_t::code>;


code_breakpoint::code_breakpoint(target_base & t, const lldb::SBBreakpoint & bp):
breakpoint_base_impl<lldb::SBBreakpoint, breakpoint_id::type_t::code>{bp},
targ_{t} {
}


code_breakpoint::~code_breakpoint() {
}


std::size_t code_breakpoint::locations_size() const {
    return lldb_bp().GetNumLocations();
}


breakpoint_location code_breakpoint::location(std::size_t i) const {
    return breakpoint_location(targ_, lldb_bp().GetLocationAtIndex(static_cast<uint32_t>(i)));
}


code_breakpoint::const_locations_iterator code_breakpoint::locations_begin() const {
    return const_locations_iterator(*this, 0);
}


code_breakpoint::const_locations_iterator code_breakpoint::locations_end() const {
    return const_locations_iterator(*this, locations_size());
}


void code_breakpoint::remove() {
    targ_.lldb_targ().BreakpointDelete(lldb_bp().GetID());
}


code_breakpoint::const_locations_iterator::const_locations_iterator(const code_breakpoint & bp, std::size_t i):
iterator_adaptor_(i),
bp_(bp) {
}


code_breakpoint::const_locations_iterator::reference code_breakpoint::const_locations_iterator::dereference() const {
    return bp_.location(this->base());
}


}
