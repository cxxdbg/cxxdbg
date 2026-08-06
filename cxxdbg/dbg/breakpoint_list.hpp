// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file breakpoint_list.hpp
/// Contains definition of the breakpoint_list class.

#pragma once

#include "breakpoint_info.hpp"
#include "code_breakpoint.hpp"
#include "watchpoint.hpp"
#include "cxxdbg/app/signals.hpp"
#include "cxxdbg/async/async.hpp"
#include <ranges.hpp>
#include <map>


namespace cxxdbg::dbg {


class code_breakpoint;
class breakpoint;
class breakpoint_list_impl;
class debugger;
class source_model;
class stack_frame;
class watchpoint;


/// List of breakpoints. Contains logic for breakpoint management
/// (adding/removing/changing) and async synchronization with impl
class breakpoint_list {
    friend class debugger;

public:
    /// Type of breakpoint number
    using num_t = breakpoint::num_t;

    /// Type of handler function for breakpoint signals
    using breakpoint_func = void (const breakpoint *);

    /// Type of handler for breakpoint signals
    using breakpoint_handler = std::function<breakpoint_func>;

    /// Type of handler for watchpoint adding functions
    using watchpoint_handler = async::result_handler<watchpoint*>;

    /// Type of handler for code breakpoint adding function
    using code_breakpoint_handler = std::function<void(const code_breakpoint*)>;


    /// Constructs breakpoint list with specified reference to source model
    /// and pointer to implementation
    breakpoint_list(source_model & src_mdl, breakpoint_list_impl * impl = nullptr):
        src_model_{src_mdl}, impl_{impl} {}

    /// Returns range of all breakpoints for target
    auto all() const {
        auto fn = [](auto && pair) { return pair.second.get(); };
        return breakpoints_ | std::ranges::views::transform(fn);
    }

    /// Returns const range of all breakpoints of specified type
    auto all_of_type(breakpoint_type type) const {
        auto ffn = [type](auto && pair) {
            return pair.first.type() == type;
        };

        auto tfn = [](auto && pair) {
            return const_cast<const breakpoint*>(pair.second.get());
        };

        return breakpoints_ | std::ranges::views::filter(ffn) | std::ranges::views::transform(tfn);
    }

    /// Returns range of all breakpoints of specified type
    auto all_of_type(breakpoint_type type) {
        auto ffn = [type](auto && pair) {
            return pair.first.type() == type;
        };

        auto tfn = [](auto && pair) {
            return const_cast<breakpoint*>(pair.second.get());
        };

        return breakpoints_ | std::ranges::views::filter(ffn) | std::ranges::views::transform(tfn);
    }

    /// Returns const range of all code breakpoints
    auto code_breakpoints() const {
        auto fn = [](auto && bp) {
            auto cbp = dynamic_cast<const code_breakpoint*>(bp);
            assert(cbp && "breakpoint is not a code breakpoint");
            return cbp;
        };

        return all_of_type(breakpoint_type::code) | std::ranges::views::transform(fn);
    }

    /// Returns range of all code breakpoints
    auto code_breakpoints() {
        auto fn = [](auto && bp) {
            auto cbp = dynamic_cast<code_breakpoint*>(bp);
            assert(cbp && "breakpoint is not a code breakpoint");
            return cbp;
        };

        return all_of_type(breakpoint_type::code) | std::ranges::views::transform(fn);
    }

    /// Returns const range of all watchpoints
    auto watchpoints() const {
        auto fn = [](auto && bp) {
            auto wp = dynamic_cast<const watchpoint*>(bp);
            assert(wp && "breakpoint is not a watchpoint");
            return wp;
        };

        return all_of_type(breakpoint_type::watch) | std::ranges::views::transform(fn);
    }

    /// Returns range of all watchpoints
    auto watchpoints() {
        auto fn = [](auto && bp) {
            auto wp = dynamic_cast<watchpoint*>(bp);
            assert(wp && "breakpoint is not a watchpoint");
            return wp;
        };

        return all_of_type(breakpoint_type::watch) | std::ranges::views::transform(fn);
    }

    /// Adds breakpoint with specified source position info
    void add_breakpoint(const source_position_info & pos,
                        const code_breakpoint_handler & h,
                        const source_position & toggle_pos = {});

    /// Adds breakpoint at function with specified name
    void add_breakpoint(const std::string & func_name, const code_breakpoint_handler & h);

    /// Adds breakpoint at specified address
    void add_breakpoint(std::uint64_t addr, const code_breakpoint_handler & h);

    /// Adds watchpoint for variable with specified name
    void add_var_watchpoint(const std::string & name,
                            bool read,
                            bool write,
                            size_t size,
                            const watchpoint_handler & handl);

    /// Adds watchpoint for address specified by expression
    void add_expr_watchpoint(const std::string & expr,
                             bool read,
                             bool write,
                             size_t size,
                             const watchpoint_handler & handl);

    /// Adds breakpoint when exception thrawn
    void add_breakpoint_exception_thrown(const code_breakpoint_handler &h);

    /// Adds breakpoint when exception caught
    void add_breakpoint_exception_caught(const code_breakpoint_handler & h);

    /// Removes specified breakpoint
    void remove_breakpoint(const breakpoint * bp);

    /// Enables specified breakpoint
    void enable_breakpoint(const breakpoint * bp);

    /// Disables specified breakpoint
    void disable_breakpoint(const breakpoint * bp);

    /// Rmoves all breakpoints
    void remove_all_breakpoints();

    /// Enables all breakpoints
    void enable_all_breakpoints();

    /// Disables all breakpoints
    void disable_all_breakpoints();

    /// Sets breakpoint condition
    void set_breakpoint_condition(const breakpoint * bp, const std::string & cond);

    /// Sets breakpoint hit count property
    void set_breakpoint_hit_count(const breakpoint * bp,
                                  bool hit_count_enabled,
                                  unsigned int hit_count,
                                  bool reset_hit_count);

    /// Returns pointer to code breakpoint with specified number or nullptr if
    /// code breakpoint not found
    const code_breakpoint * find_code_breakpoint(breakpoint_num n) const;

    /// Returns pointer to watchpoint with specified number or nullptr if
    /// watchpoint not found
    const watchpoint * find_watchpoint(breakpoint_num n) const;

    /// Returns breakpoint with specified type and number. Returns nullptr
    /// if breakpoint not found
    const breakpoint * find_breakpoint(const breakpoint_id & id) const;

    /// Resets current hit count for all breakpoints to zero, sets
    /// new ignore count values for all breakpoints in implementation
    void reset_breakpoints();

    /// Updates breakpoints states according to specified vector of
    /// breakpoint infos
    void update_breakpoints(const std::vector<std::shared_ptr<breakpoint_info>> & bps,
                            bool is_stop_reason_wp,
                            const stack_frame * curr_frame);

    /// Toggles breakpoint at specified source position
    void toggle_breakpoint(const source_file * src, unsigned int line);

    /// The signal is emitted after breakpoint added
    CXXDBG_DEFINE_SIGNALX(added, void (const breakpoint *))

    /// The signal is emitted after breakpoint removed
    CXXDBG_DEFINE_SIGNALX(removed, void (const breakpoint*))

    /// The signal is emitted after breakpoint changed
    CXXDBG_DEFINE_SIGNALX(changed, void (const breakpoint*))

    /// The signal is emitted after any breakpoint in list added/removed/changed
    CXXDBG_DEFINE_SIGNALX(list_changed, void())

    /// The signal is emitted after adding breakpoint via toggle if actual
    /// breakpoint location differs from location requested, and client
    /// should change position in document
    CXXDBG_DEFINE_SIGNALX(toggle_pos_changed, void (const source_position &))

private:       
    /// Connects breakpoint list to implementation and installs all existing
    /// breakpoints into it
    void connect_impl(breakpoint_list_impl * impl);

    /// Disconnects this list from implementation
    void disconnect_impl();

    /// Adds breakpoint locations from bp info to breakpoint object
    void add_breakpoint_locations(code_breakpoint * bp,
                                  const code_breakpoint_info & bp_inf);

    /// Returns current next breakpoint number and increases it
    num_t next_bp_num();

    /// Returns current next watchpoint number and increases it
    num_t next_wp_num();

    /// Calculates breakpoint ID for specified breakpoint
    breakpoint_id bp_id(const breakpoint * bp);

    /// Adds code breakpoint into list and installs it into implementation
    /// if implementation is connected
    void add_code_breakpoint(const std::shared_ptr<code_breakpoint> & bp,
                             const code_breakpoint_handler & h,
                             const source_position & toggle_pos = {});

    /// Installs code breakpoint into implementation
    void install_code_breakpoint_into_impl(code_breakpoint * bp,
                                           const source_position & toggle_pos = {});

    source_model & src_model_;          ///< Reference to source model
    breakpoint_list_impl * impl_;       ///< Pointer to implementation

    num_t next_bp_num_ = 1;             ///< Next breakpoint number
    num_t next_wp_num_ = 1;             ///< Next watchpoint number

    /// Breakpoint map
    std::map<breakpoint_id, std::shared_ptr<breakpoint>> breakpoints_;
};


}


