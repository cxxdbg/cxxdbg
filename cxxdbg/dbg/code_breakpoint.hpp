// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file code_breakpoint.hpp
/// Contains definition of code_breakpoint class.

#pragma once

#include "breakpoint.hpp"
#include "breakpoint_site.hpp"
#include "code_position.hpp"

#include <ranges.hpp>
#include <functional>
#include <list>


namespace cxxdbg::dbg {


class breakpoint_list;
class breakpoint_location;
class code_breakpoint_info;
class source_model;


struct code_breakpoint_get_ptr {
    template <typename SmartPtr>
    auto operator()(SmartPtr && ptr) const {
        return const_cast<const breakpoint_location*>(ptr.get());
    }
};


struct code_breakpoint_make_loc_range {
    template <typename Vector>
    auto operator()(Vector & v) const {
        return v | std::ranges::views::transform(code_breakpoint_get_ptr());
    }
};


/// \class breakpoint
/// Represents single breakpoint in code
class code_breakpoint: public breakpoint {
    friend class breakpoint_list;

    /// Type of shared pointer to breakpoint location
    typedef std::shared_ptr<breakpoint_location> breakpoint_location_ptr;

    /// Type of vector of breakpoint locations
    typedef std::list<breakpoint_location_ptr> breakpoint_location_list;

public:
    /// Type of breakpoint ID
    typedef int id_type;

    /// Constructor, makes breakpoint with specified reference t
    /// source model and ID
    code_breakpoint(source_model & src_mdl, num_t n);

    /// Destructor, destroys object
    virtual ~code_breakpoint() override;

    /// Returns number of breakpoint locations
    std::size_t locations_size() const;

    /// Returns range of breakpoint locations
    const auto & locations() const { return locations_r_; }

    /// Adds breakpoint location to breakpoint
    void add_location(const breakpoint_location_ptr & loc);

    /// Removes breakpoint location with specified ID
    breakpoint_location_ptr remove_location(id_type id);

    /// Returns breakpoint location with specified ID
    breakpoint_location * find_location(id_type id);

    /// Updates breakpoint with data contained in breakpoint info.
    /// Returns true if breakpoint was changed
    bool update(const breakpoint_info * bpinf) override;

    ////////////////////////////////////////////////////////////////////////////////
    // Implementation of breakpoint_site interface

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

protected:
    using install_handler = std::function<void (const code_breakpoint_info &)>;

private:
    /// Removes all locations from breakpoint
    void remove_all_locations();

    // non copyable
    code_breakpoint(const code_breakpoint &) = delete;
    code_breakpoint & operator=(const code_breakpoint &) = delete;

    /// Installs code breakpoint into implementation.
    /// Should be called for existing breakpoints after target load.
    /// Invokes handler with breakpoint info after install is finished
    virtual void install_into_impl(breakpoint_list_impl & impl, const install_handler & h) = 0;


    source_model & src_mdl_;                ///< Reference to source model
    breakpoint_location_list locations_;    ///< Breakpoint locations

    /// Range of pointers to breakpoint locations
    decltype(code_breakpoint_make_loc_range()(locations_)) locations_r_ = code_breakpoint_make_loc_range()(locations_);
};


}


