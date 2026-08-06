// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file breakpoint_base_impl.hpp
/// Contains definition of the breakpoint_base_impl class.

#pragma once

#include "breakpoint.hpp"


namespace cxxdbg::dbg::core {


/// Base class of implementation for breakpoints and watchpoints. Contains
/// functions common for both breakpoints and watchpoints
template <typename LLDBType, breakpoint_id::type_t IdType>
class breakpoint_base_impl: public breakpoint {
public:
    /// Constructs invalid breakpoint
    breakpoint_base_impl() = default;

    /// Destroys object
    ~breakpoint_base_impl() = default;

    /// Constructs breakpoint object with specified LLDB representation object
    explicit breakpoint_base_impl(LLDBType lldbb):
        breakpoint{{IdType, static_cast<breakpoint_id::num_t>(lldbb.GetID())}},
        lldb_bp_{lldbb} {}

    /// Returns breakpoint condition
    std::string condition() const override {
        const char * cond = lldb_bp().GetCondition();
        return cond != nullptr ? std::string(cond) : std::string();
    }

    /// Returns ignore count property
    unsigned int ignore_count() const override {
        return lldb_bp().GetIgnoreCount();
    }

    /// Returns current hit count
    unsigned int curr_hit_count() const override {
        return lldb_bp().GetHitCount();
    }

    /// Returns true if breakpoint is enabled
    bool enabled() const override {
        return lldb_bp().IsEnabled();
    }

    /// Enables breakpoint
    void enable() override {
        lldb_bp().SetEnabled(true);
    }

    /// Disables breakpoint
    void disable() override {
        lldb_bp().SetEnabled(false);
    }

    /// Sets breakpoint condition
    void set_condition(const std::string & cond) override {
        lldb_bp().SetCondition(cond.c_str());
    }

    /// Sets breakpoint ignore count
    void set_ignore_count(unsigned int cnt) override {
        lldb_bp().SetIgnoreCount(cnt);
    }

    /// Returns true if breakpoint is valid
    bool is_valid() const {
        return lldb_bp().IsValid();
    }

    /// Returns true if breakpoint is valid
    operator bool() const {
        return is_valid();
    }

    /// Returns true if breakpoint is invalid
    bool operator!() const {
        return !is_valid();
    }

protected:
    /// Returns LLDB representation of breakpoint
    LLDBType lldb_bp() const { return lldb_bp_; }

private:
    LLDBType lldb_bp_;      ///< LLDB breakpoint
};


}
