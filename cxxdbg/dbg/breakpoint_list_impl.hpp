// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file breakpoint_list_impl.hpp
/// Contains definition of the breakpoint_list_impl class.

#pragma once

#include "breakpoint_info.hpp"
#include "cxxdbg/async/async.hpp"


namespace cxxdbg::dbg {


/// Interface for breakpoint list implementation. Contains virtual functions
/// for managing breakpoint in debug target.
class breakpoint_list_impl {
public:
    /// Virtual destructor
    virtual ~breakpoint_list_impl() = default;

    /// Type of handler for code breakpoint adding functions
    using code_breakpoint_handler = std::function<void (const code_breakpoint_info &)>;

    /// Adding watchpoint completion handler
    using watchpoint_handler = async::result_handler<watchpoint_info_sp>;

    /// Adds breakpoint with specified source position
    virtual void add_breakpoint(const source_position_info & pos,
                                bool is_enabled,
                                const std::string & condition,
                                const code_breakpoint_handler & handl) = 0;

    /// Adds breakpoint at function with specified name
    virtual void add_breakpoint(const std::string & func_name,
                                bool is_enabled,
                                const std::string & condition,
                                const code_breakpoint_handler & handl) = 0;

    /// Adds breakpoint at specified address
    virtual void add_breakpoint(std::uint64_t addr,
                                bool is_enabled,
                                const std::string & condition,
                                const code_breakpoint_handler & handl) = 0;

    /// Removes breakpoint with specified id
    virtual void remove_breakpoint(const breakpoint_id & id) = 0;

    /// Enables breakpoint with specified id
    virtual void enable_breakpoint(const breakpoint_id & id) = 0;

    /// Disables breakpoint with specified id
    virtual void disable_breakpoint(const breakpoint_id & id) = 0;

    /// Sets condition for breakpoint with specified id
    virtual void set_breakpoint_condition(const breakpoint_id & id, const std::string & cond) = 0;

    /// Sets ignore count for breakpoint with specified id
    virtual void set_breakpoint_ignore_count(const breakpoint_id & id, unsigned int cnt) = 0;

    /// Adds watchpoint for variable with specified name
    virtual void add_var_watchpoint(const std::string & name,
                                    bool read,
                                    bool write,
                                    size_t size,
                                    const watchpoint_handler & handl) = 0;

    /// Adds watchpoint for address specified by expression
    virtual void add_expr_watchpoint(const std::string & expr,
                                     bool read,
                                     bool write,
                                     size_t size,
                                     const watchpoint_handler & handl) = 0;
};


}


