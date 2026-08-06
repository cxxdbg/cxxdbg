// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file breakpoint_info.hpp
/// Contains definition of breakpoint_info class.

#pragma once

#include "breakpoint_location_info.hpp"
#include "cxxdbg/dbg/core/breakpoint_id.hpp"
#include <vector>


namespace cxxdbg::dbg {


/// Type of breakpoint type
using breakpoint_type = core::breakpoint_type;

/// Type of breakpoint number
using breakpoint_num = core::breakpoint_num;

/// Type of breakpoint ID
using breakpoint_id = core::breakpoint_id;


/// Base class for breakpoint and watchpoint info
class breakpoint_info {
public:
    /// Type of breakpoint type
    using type_t = core::breakpoint_type;

    /// Type of breakpoint number
    using num_t = core::breakpoint_num;

    /// Type of breakpoint ID
    using id_t = core::breakpoint_id;

    /// Constructs breakpoint info with specified number
    breakpoint_info(const num_t & n):
        num_{n}, curr_hit_count_{0} {}

    /// Virtual destructor. Required for generating RTTI (for dyn cast)
    virtual ~breakpoint_info() = default;

    /// Returns breakpoint number
    auto num() const { return num_; }

    /// Returns breakpoint type
    virtual breakpoint_type type() const = 0;

    /// Returns breakpoint ID
    breakpoint_id id() const { return {type(), num()}; }

    /// Returns current hit count
    auto curr_hit_count() const { return curr_hit_count_; }

    /// Sets current hit count
    void set_curr_hit_count(unsigned int hc) { curr_hit_count_ = hc; }

private:
    num_t num_;                         ///< Breakpoint number
    unsigned int curr_hit_count_;       ///< Breakpoint current hit count
};


/// \class breakpoint_info
/// Contains information about breakpoint in target being debugged
class code_breakpoint_info: public breakpoint_info {
public:
    /// Constructs new breakpoint info
    code_breakpoint_info(num_t n): breakpoint_info{n} {}

    /// Returns breakpoint type
    breakpoint_type type() const override { return type_t::code; }

    /// Return range of location infos
    auto & locations() const { return locations_; }

    /// Adds breakpoint location
    void add_location(const breakpoint_location_info & loc);

private:
    std::vector<breakpoint_location_info> locations_;     ///< Breakpoint locations
};


/// Watchpoint info
class watchpoint_info: public breakpoint_info {
public:
    /// Constructs watchpoint info with specified number
    watchpoint_info(num_t n): breakpoint_info{n} {}

    /// Returns breakpoint type
    breakpoint_type type() const override { return type_t::watch; }

    /// Returns old watchpoint value
    auto & old_val() const { return old_val_; }

    /// Sets old watchpoint value
    void set_old_val(const std::wstring & ov) { old_val_ = ov; }

    /// Returns new watchpoint value
    auto & new_val() const { return new_val_; }

    /// Sets new watchpoint value
    void set_new_val(const std::wstring & nv) { new_val_ = nv; }

    /// Returns watch address
    auto watch_addr() const { return watch_addr_; }

    /// Sets watch address
    void set_watch_addr(uint64_t wa) { watch_addr_ = wa; }

private:
    std::wstring old_val_;      ///< Old watchpoint value
    std::wstring new_val_;      ///< New watchpoint value
    uint64_t watch_addr_;       ///< Watch address
};


/// Type of shared pointer to watchpoint info
using watchpoint_info_sp = std::shared_ptr<watchpoint_info>;


}


