// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file process_context.hpp
/// Contains definition of process_context class.

#pragma once

#include "forward.hpp"
#include "breakpoint_info.hpp"
#include "thread_list_info.hpp"
#include "watch_list_impl.hpp"


namespace cxxdbg::dbg {


/// \class process_context
/// Represents state of threads and breakpoints of process in stopped state.
/// The class is used to pass state from implementation to app when process stops
class process_context {
    /// Type of shared pointer to breakpoint info
    typedef std::shared_ptr<breakpoint_info> breakpoint_info_sp;

    /// Type of list of breakpoint infos
    typedef std::vector<breakpoint_info_sp> breakpoint_info_vector;

    /// Type of vector of module infos
    typedef std::vector<module_info> module_info_vector;

    /// Type of shared pointer to vector of module infos
    typedef std::shared_ptr<module_info_vector> module_info_vector_ptr;


public:
    /// Target stop reason
    enum stop_reason_t {
        stop_reason_unknown,        ///< Unknown stop reason
        stop_reason_trace,          ///< Tracing
        stop_reason_breakpoint,     ///< Breakpoint
        stop_reason_watchpoint,     ///< Watchpoint
        stop_reason_signal,         ///< Signal received
        stop_reason_exec,           ///< exec called
        stop_reason_crashed         ///< Process crashed
    };

    /// Constructor, constructs empty process context
    process_context();

    /// Returns ID of process
    unsigned long pid() const;

    /// Sets ID of process
    void set_pid(unsigned long p);

    /// Returns reference to thread info
    thread_list_info & threads();

    /// Returns const reference to thread info
    const thread_list_info & threads() const;

    /// Returns reference to breakpoints info
    breakpoint_info_vector & breakpoints();

    /// Returns const reference to breakpoints info
    const breakpoint_info_vector & breakpoints() const;

    /// Returns reference to locals tree
    watch_list_impl::tree_info & locals();

    /// Returns const reference to locals tree
    const watch_list_impl::tree_info & locals() const;

    /// Sets locals tree
    void set_locals(const watch_list_impl::tree_info & ltree);

    /// Returns const reference to watch tree
    const watch_list_impl::tree_info & watch() const;

    /// Sets watch tree
    void set_watch(const watch_list_impl::tree_info & wtree);

    /// Returns const reference to registers tree
    const auto & registers() const { return registers_; }

    /// Sets registers tree
    void set_registers(const watch_list_impl::tree_info & rtree) { registers_ = rtree; }

    /// Returns stop reason
    stop_reason_t stop_reason() const;

    /// Sets stop reason
    void set_stop_reason(stop_reason_t rsn);

    /// Returns stop signal
    int stop_signal() const;

    /// Sets stop signal
    void set_stop_signal(int s);

    /// Returns active watchpoint number
    breakpoint_num watchpoint_num() const { return active_wp_; }

    /// Sets active watchpoint number
    void set_watchpoint_num(breakpoint_num n) { active_wp_ = n; }

    /// Returns pointer to vector of updated modules info
    module_info_vector_ptr modules() const;

    /// Sets pointer to vector of updated modules info
    void set_modules(const module_info_vector_ptr & mods);

private:
    unsigned long pid_;                         ///< ID of process
    stop_reason_t stop_rsn_;                    ///< Stop reason
    int stop_signal_;                           ///< Stop signal number or zero
    breakpoint_num active_wp_;                  ///< Number of active watchpoint
    thread_list_info threads_;                  ///< Threads info
    breakpoint_info_vector breakpoints_;        ///< Breakpoints info
    watch_list_impl::tree_info locals_;         ///< Local variables watch tree
    watch_list_impl::tree_info watch_;          ///< Custom watch tree
    watch_list_impl::tree_info registers_;      ///< Registers watch tree
    module_info_vector_ptr modules_;            ///< Optional info about updated modules
};


}


