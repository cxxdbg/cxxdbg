// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file breakpoint.cpp
/// Contains implementation of breakpoint class.

#include "code_breakpoint.hpp"
#include "breakpoint_info.hpp"
#include "breakpoint_location.hpp"
#include "target.hpp"
#include <ranges.hpp>
#include <cassert>
#include <sstream>


namespace cxxdbg::dbg {


code_breakpoint::code_breakpoint(source_model & src_mdl, num_t n):
breakpoint{n},
src_mdl_{src_mdl} {
}


code_breakpoint::~code_breakpoint() {
}


std::size_t code_breakpoint::locations_size() const {
    return locations_.size();
}


void code_breakpoint::add_location(const breakpoint_location_ptr & loc) {
    locations_.push_back(loc);
}


code_breakpoint::breakpoint_location_ptr code_breakpoint::remove_location(id_type id) {
    for (auto it = locations_.begin(), end = locations_.end(); it != end; ++it) {
        if ((*it)->id() == id) {
            breakpoint_location_ptr loc = *it;
            locations_.erase(it);
            return loc;
        }
    }

    assert(false && "breakpoint location is not found");
    return nullptr;
}


breakpoint_location * code_breakpoint::find_location(id_type id) {
    for (auto it = locations_.begin(), end = locations_.end(); it != end; ++it) {
        if ((*it)->id() == id) {
            return it->get();
        }
    }

    assert(false && "breakpoint location is not found");
    return nullptr;
}


bool code_breakpoint::update(const breakpoint_info * bpinf) {

    auto cbpinf = dynamic_cast<const code_breakpoint_info*>(bpinf);
    assert(cbpinf && "Invalid breakpoint info type passed to code_breakpoint::update");

    bool changed = false;

    // updating base properties
    changed |= breakpoint::update(cbpinf);

    // removing locations
    // TODO: remove n^2 complexity?
    auto old_size = locations_.size();
    locations_.remove_if([cbpinf](auto && loc) {
        return std::ranges::find_if(cbpinf->locations(), [&loc](auto && loc_inf) {
            return loc_inf.id() == loc->id();
        }) == std::ranges::end(cbpinf->locations());
    });

    changed |= (old_size != locations_.size());

    // adding new locations and updating existing
    // TODO: remove n^2 complexity?
    for (auto loc_inf : cbpinf->locations()) {
        auto res = std::ranges::find_if(locations_, [&loc_inf](auto && loc) {
            return loc->id() == loc_inf.id();
        });

        if (res == locations_.end()) {
            // new location
            auto pos = target::convert_code_position_info(src_mdl_, loc_inf.pos());
            std::shared_ptr<breakpoint_location> loc{new breakpoint_location{this, loc_inf.id(), pos}};
            locations_.push_back(loc);
            changed = true;

        } else {
            // updating existing location
            auto pos = target::convert_code_position_info(src_mdl_, loc_inf.pos());
            if ((*res)->pos() != pos) {
                (*res)->set_pos(pos);
                changed = true;
            }
        }
    }

    return changed;
}

bool code_breakpoint::can_change_enabled_state() const {
    return true;
}

const breakpoint_location * code_breakpoint::get_single_location() const {
    if (locations_.size() == 1) {
        return locations_.front().get();
    }

    return nullptr;
}

std::size_t code_breakpoint::children_size() const {
    return locations_.size();
}

const breakpoint_site * code_breakpoint::child_at(std::size_t position) const {
    assert(position < children_size() && "position out of range");
    std::vector<int> v;

    auto it = locations_.begin();
    for (std::size_t pos = 0; pos < position; ++pos) {
        ++it;   // cant make it + position for std::list
        // std::advance is not available until C++17
    }

    return it->get();
}

source_position code_breakpoint::get_source_position() const {
    if (children_size() == 0 || children_size() > 1)
        return {};

    auto * location = child_at(0);

    return location->get_source_position();
}


void code_breakpoint::remove_all_locations() {
    locations_.clear();
}


}
