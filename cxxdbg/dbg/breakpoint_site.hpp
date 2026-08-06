// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

//
// Created by yzarudny on 28.10.17.
//

#pragma once

#include <string>
#include <memory>
#include "code_position.hpp"

namespace cxxdbg::dbg {

class breakpoint_location;

/// @class breakpoint_site is a base class for breakpoints and breakpoint locations
class breakpoint_site {
public:
//    typedef std::shared_ptr<breakpoint_site> breakpoint_site_sp;
    virtual ~breakpoint_site() {}

//    /// removes all children
//    virtual void remove_all_children() = 0;

    /// @return child at position
    virtual const breakpoint_site * child_at(std::size_t position) const = 0;

    /// @return number of children
    virtual std::size_t children_size() const = 0;

    /// @return essential location
    virtual const breakpoint_location * get_single_location() const = 0;

    /// bp locations cannot be enabled or disabled yet
    /// TODO: allow locations to be enabled or disabled
    virtual bool can_change_enabled_state() const { return true; };

    /// returns code position for goto source action
    virtual source_position get_source_position() const = 0;
};

}

