// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file breakpoint.hpp
/// Contains definition of the breakpoint class.

#pragma once

#include "breakpoint_info.hpp"
#include "breakpoint_site.hpp"
#include <memory>


namespace cxxdbg::dbg {

class breakpoint_list;
class breakpoint_list_impl;


/// Represents abstract breakpoint in target (in code or watchpoint)
class breakpoint: virtual public breakpoint_site {
    friend class breakpoint_list;

public:
    /// Type of breakpoint number
    using num_t = breakpoint_num;

    /// Invalid number value
    static constexpr num_t invalid_num = static_cast<num_t>(-1);

    // noncopyable
    breakpoint(const breakpoint &) = delete;
    breakpoint& operator=(const breakpoint&) = delete;

    /// Constructs breakpoint with specified number
    explicit breakpoint(num_t n):
        num_{n}, impl_num_{invalid_num}, enabled_{true}, hit_count_enabled_{false}, hit_count_{1},
        curr_hit_count_{0} {}

    /// Destroys object
    virtual ~breakpoint() = default;

    /// Returns breakpoint name
    virtual std::string name() const = 0;

    /// Returns breakpoint number (not necessary unique)
    num_t num() const;

    /// Returns true if breakpoint is enabled
    bool enabled() const;

    /// Sets enable flag for breakpoint
    void set_enabled(bool val);

    /// Returns breakpoint condition
    const std::string & condition() const;

    /// Sets breakpoint condition
    void set_condition(const std::string & cond);

    /// Returns true if hit count condition is enabled
    bool hit_count_enabled() const;

    /// Sets whether hit count condition is enabled
    void set_hit_count_enabled(bool val);

    /// Returns hit count property
    unsigned int hit_count() const;

    /// Sets hit count property for breakpoint
    void set_hit_count(unsigned int hc);

    /// Returns current hit count
    unsigned int curr_hit_count() const;

    /// Resets current hit count to zero
    void reset_curr_hit_count();

    /// Calculates ignore count value based on enabled/disable
    /// hit count property and current hit count
    unsigned int ignore_count() const;

    /// Updates breakpoint. Returns true if breakpoint was changed
    virtual bool update(const breakpoint_info * bpinf);

    /// Converts breakpoint hit count property and current hit count to string
    std::string hit_count_str() const;

protected:
    /// Called before target executable starts. All required before
    /// start actions should be performed in that function. Default
    /// implmentation resets current hit counter
    virtual void before_start();

    /// Returns breakpoint number in implementation
    num_t impl_num() const { return impl_num_; }

    /// Sets breakpoint implementation number
    void set_impl_num(num_t n) {
        impl_num_ = n;
    }

    num_t num_;                             ///< Breakpoint number
    num_t impl_num_;                        ///< Breakpoint number in implementation
    bool enabled_;                          ///< true if breakpoint is enabled
    std::string condition_;                 ///< Breakpoint condition
    bool hit_count_enabled_;                ///< Is hit count condition enabled?
    unsigned int hit_count_;                ///< Breakpoint hit count property
    unsigned int curr_hit_count_;           ///< Breakpoint current hit count
};


/// Type of shared pointer to breakpoint
using breakpoint_sp = std::shared_ptr<breakpoint>;


}


