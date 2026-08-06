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
#include "breakpoint_site.hpp"


namespace cxxdbg::dbg {


class code_breakpoint;


/// \class breakpoint_location
/// Represents single breakpoint location in target being debugged
class breakpoint_location: public breakpoint_site {
public:
    /// Type of breakpoint location ID
    typedef unsigned long id_type;

    /// Constructor, makes new breakpoint location with specified pointer
    /// to breakpoint, location ID, and code position
    breakpoint_location(code_breakpoint * bp, id_type i, const code_position & p);

    /// Destructor, destroys object
    ~breakpoint_location() override;

    /// Returns const pointer to breakpoint
    const code_breakpoint * bp() const;

    /// Returns pointer to breakpoint
    code_breakpoint * bp();

    /// Returns location ID
    id_type id() const;

    /// Returns code position for breakpoint location
    const code_position & pos() const;

    /// Sets code position for breakpoint location
    void set_pos(const code_position & p);

    ////////////////////////////////////////////////////////////////////////////////
    // implementation of breakpoint_site

    /// @return child at position
    const breakpoint_site * child_at(std::size_t position) const override;

    /// @return number of children
    std::size_t children_size() const override;

    /// @return essential location
    const breakpoint_location * get_single_location() const override;

    /// bp locations cannot be enabled or disabled yet
    /// TODO: allow locations to be enabled or disabled
    bool can_change_enabled_state() const override;

    /// returns code position for goto source action
    source_position get_source_position() const override;

private:
    // non copyable
    breakpoint_location(const breakpoint_location &) = delete;
    breakpoint_location & operator=(const breakpoint_location &) = delete;

    code_breakpoint * bp_;       ///< Pointer to breakpoint
    id_type id_;            ///< Breakpoint location ID
    code_position pos_;     ///< Code position for location
};


}


