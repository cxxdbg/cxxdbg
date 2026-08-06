// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file breakpoint.hpp
/// Contains definition of the breakpoint class.

#pragma once

#include "breakpoint_id.hpp"
#include <memory>
#include <string>


namespace cxxdbg::dbg::core {


class target_base;


/// Base class of breakpoints and watchpoints
class breakpoint {
    friend class target_base;

public:
    /// Breakpoint ID type
    using id_t = breakpoint_id;

    /// Breakpoint type
    using type_t = id_t::type_t;

    /// Constructs breakpoint with specified ID
    breakpoint(id_t i): id_{i} {}

    /// Destroys object
    virtual ~breakpoint() = default;

    /// Returns breakpoint ID
    auto id() const { return id_; }

    /// Returns breakpoint condition
    virtual std::string condition() const = 0;

    /// Returns ignore count property
    virtual unsigned int ignore_count() const = 0;

    /// Returns current hit count
    virtual unsigned int curr_hit_count() const = 0;

    /// Returns true if breakpoint is enabled
    virtual bool enabled() const = 0;

    /// Enables breakpoint
    virtual void enable() = 0;

    /// Disables breakpoint
    virtual void disable() = 0;

    /// Sets breakpoint condition
    virtual void set_condition(const std::string & cond) = 0;

    /// Sets breakpoint ignore count
    virtual void set_ignore_count(unsigned int cnt) = 0;

private:
    /// Removes breakpoint from target
    virtual void remove() = 0;

    id_t id_;               ///< Breakpoint ID
};


/// Shared pointer to breakpoint
using breakpoint_sp = std::shared_ptr<breakpoint>;


}
