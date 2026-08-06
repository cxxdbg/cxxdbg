// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file watchpoint.cpp
/// Contains implementation of the watchpoint class.

#include "watchpoint.hpp"

#include <cassert>
#include <sstream>
#include <iostream>


namespace cxxdbg::dbg {


void watchpoint::before_start() {
    breakpoint::before_start();

    /// lldb resets current hit count for watchpoints after restart,
    /// so we should reset current real hit count in base class
    reset_real_curr_hit_count();
}

watchpoint::watchpoint(breakpoint_num n, bool read, bool write, size_t size) :
        breakpoint{n},
        read_{read}, write_{write}, size_{size},
        address_(0) {
}

bool watchpoint::update(const breakpoint_info * bpinf) {
    bool changed = breakpoint::update(bpinf);

    auto * wpInfo = dynamic_cast<const watchpoint_info*>(bpinf);
    assert(wpInfo != nullptr);

    if (old_val() != wpInfo->old_val()) {
        changed = true;
        set_old_val(wpInfo->old_val());
    }

    if (new_val() != wpInfo->new_val()) {
        changed = true;
        set_new_val(wpInfo->new_val());
    }

    if (address() != wpInfo->watch_addr()) {
        changed = true;
        set_address(wpInfo->watch_addr());
    }

    return changed;
}

const breakpoint_site * watchpoint::child_at(std::size_t position) const {
    assert(false && "watchpoints have no children");
    return nullptr;
}

std::size_t watchpoint::children_size() const {
    return 0;
}

const breakpoint_location * watchpoint::get_single_location() const {
    // no locations, GoTo source action is not enabled for watchpoints
    return nullptr;
}

source_position watchpoint::get_source_position() const {
    return pos_;
}

variable_watchpoint::variable_watchpoint(breakpoint_num n, bool read, bool write, size_t size, std::string name) :
        watchpoint{n, read, write, size},
        name_{std::move(name)} {
}


expression_watchpoint::expression_watchpoint(breakpoint_num n, bool read, bool write, size_t size,
                                             std::string expression) :
        watchpoint{n, read, write, size},
        expression_{std::move(expression)} {
}


}
