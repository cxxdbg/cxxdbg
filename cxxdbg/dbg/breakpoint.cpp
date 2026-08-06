// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file breakpoint.cpp
/// Contains implementation of the breakpoint class.

#include "breakpoint.hpp"
#include "breakpoint_info.hpp"
#include <cassert>
#include <sstream>


namespace cxxdbg::dbg {


constexpr breakpoint::num_t breakpoint::invalid_num;


breakpoint::num_t breakpoint::num() const {
    return num_;
}


bool breakpoint::enabled() const {
    return enabled_;
}


void breakpoint::set_enabled(bool val) {
    enabled_ = val;
}


const std::string & breakpoint::condition() const {
    return condition_;
}


void breakpoint::set_condition(const std::string & cond) {
    condition_ = cond;
}


bool breakpoint::hit_count_enabled() const {
    return hit_count_enabled_;
}


void breakpoint::set_hit_count_enabled(bool val) {
    hit_count_enabled_ = val;
}


void breakpoint::set_real_curr_hit_count(unsigned int hc) {
    assert(hc >= real_curr_hit_count_);

    // increasing current hit count value
    curr_hit_count_ += (hc - real_curr_hit_count_);

    real_curr_hit_count_ = hc;
}


unsigned int breakpoint::hit_count() const {
    return hit_count_;
}


void breakpoint::set_hit_count(unsigned int hc) {
    hit_count_ = hc;
}


unsigned int breakpoint::curr_hit_count() const {
    return curr_hit_count_;
}


void breakpoint::reset_curr_hit_count() {
    curr_hit_count_ = 0;
}


unsigned int breakpoint::ignore_count() const {
    if (!hit_count_enabled()) {
        return 0;
    }

    if (curr_hit_count() >= hit_count()) {
        return 0;
    }

    return hit_count() - curr_hit_count() - 1;
}


bool breakpoint::update(const breakpoint_info * bpinf) {

    // updating hit count
    bool changed = false;
    assert(bpinf->curr_hit_count() >= real_curr_hit_count_);
    unsigned int d = bpinf->curr_hit_count() - real_curr_hit_count_;
    if (d != 0) {
        // increasing current hit count value
        curr_hit_count_ += d;
        real_curr_hit_count_ = bpinf->curr_hit_count();

        changed = true;
    }

    return changed;
}


void breakpoint::before_start() {
    curr_hit_count_ = 0;
}

std::string breakpoint::hit_count_str() const {
    std::ostringstream str;

    if (!hit_count_enabled()) {
        str << "Break always";
    } else {
        str << "Break if >= " << hit_count();
    }

    str << " (currently " << curr_hit_count() << ")";
    return str.str();
}


}
