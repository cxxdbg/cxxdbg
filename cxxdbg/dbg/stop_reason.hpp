// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file stop_reason.hpp
/// Contains definitions of stop reason classes.

#pragma once

#include "breakpoint_info.hpp"
#include <string>


namespace cxxdbg::dbg {


/// \class stop_reason
/// Base class of all stop reasons
class stop_reason {
public:
    /// Destructor, destroys object
    virtual ~stop_reason();

    /// Returns true if stop reason is trace
    bool is_trace() const;

    /// Returns true if stop reason is breakpoint
    bool is_break() const;

    /// Returns true if stop reason is watchpoint
    bool is_wpoint() const;
    
    /// Returns true if stop reason is signal
    bool is_signal() const;

    /// Returns true if stop reasong is exec
    bool is_exec() const;

    /// Returns true if stop reason is crash
    bool is_crash() const;
};


/// Stop reason after trace command
class stop_reason_trace: public stop_reason {
public:
    /// Destructor, destroys object
    ~stop_reason_trace() override;
};


/// Stop reason at breakpoint
class stop_reason_breakpoint: public stop_reason {
public:
    /// Destructor, destroys object
    ~stop_reason_breakpoint() override;
};


/// Stop reason describing stop at watchpoint
class stop_reason_watchpoint: public stop_reason {
public:
    /// Constructs stop reason with specified number of active watchpoint
    stop_reason_watchpoint(breakpoint_num wpn):
        wp_num_{wpn} {}

    /// Returns number of active watchpoint
    auto watchpoint_num() const { return wp_num_; }

private:
    breakpoint_num wp_num_;
};


/// Stop rason on signal
class stop_reason_signal: public stop_reason {
public:
    /// Constructor, makes stop reason with specified signal number
    stop_reason_signal(int s);

    /// Destructor, destroys object
    ~stop_reason_signal() override;

    /// Returns signal number which process received
    int sig_num() const;

    /// Retrns name of signal which process received
    std::string sig_name() const;

private:
    int sig_num_;       ///< Received signal number
};


/// Stop reason on exec
class stop_reason_exec: public stop_reason {
public:
    /// Destructor, destroys object
    ~stop_reason_exec() override;
};


/// Stop reason on crash
class stop_reason_crash: public stop_reason {
public:
    /// Destructor, destroys object
    ~stop_reason_crash() override;
};


}


