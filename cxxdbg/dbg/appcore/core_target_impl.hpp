// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file core_target_impl.hpp
/// Contains definition of core_target_impl class.

#pragma once

#include "core_custom_watch_list_impl.hpp"
#include "core_locals_list_impl.hpp"
#include "registers_list_impl.hpp"
#include "cxxdbg/dbg/target_impl.hpp"
#include "cxxdbg/dbg/core/target.hpp"
#include <functional>
#include <set>


namespace cxxdbg::dbg::appcore {


/// \class core_target_impl
/// Incapsulates implementation of async data fetching from target
class core_target_impl: public dbg::target_impl {
public:
    /// Constructor, makes new target implementation with specified
    /// reference to debugger instance, reference to
    /// main thread event queue, reference to core async execution queue,
    /// and pointer to core target
    core_target_impl(core::debugger & dbg,
                     async::event_queue & m_q,
                     async::execution_queue & c_a_q,
                     const std::shared_ptr<core::target> & tgt,
                     const std::vector<dbg::module_info> & mods);

    /// Destructor, destroys object
    virtual ~core_target_impl() override;


    //////////////////////////////////////////////////
    // Target state

    /// Connects to state changed signal
    signal_connection connect_state_changed(const state_changed_handler & handl) override;

    /// Returns target implementation state
    state_t state() const override;

    /// Starts launching target with specified working directory,
    /// command arguments, and result handler. May be called only in loaded
    /// state. After calling, the state of target becomes launching.
    void launch(const dbg::launch_options & lopts,
                const async::result_handler<unsigned long> & handler) override;

    /// Starts attaching target to process with specified process
    /// id or name. May be called only in loaded state. After calling, the
    /// state of target becomes launching.
    void attach(const std::variant<unsigned long, std::string> & targ,
                const async::result_handler<> & h) override;

    /// Continue execution of target. May be called in stopped state.
    /// After calling, the state of target becomes running.
    void run() override;

    /// Starts stopping target process. May be called only in running
    /// or stopped state. After calling, the state of target does not change
    /// before actual stopping of target.
    void stop() override;

    /// Starts terminating target process. May be called only in stopped
    /// state. After calling, the state of target becomes terminating.
    void terminate() override;

    /// Starts detaching from target process. May be called only in
    /// stopped state. After calling, the state of target becomes detaching.
    void detach() override;

    /// Deletes target and makes it invalid. Can be called only in
    /// 'loaded' state. Calls specified handler after deleting target.
    void close(const std::function<void()> & handl) override;

    /// Performs "step into" action on target. Can be called only in
    /// 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    void step_into(unsigned long thread_id,
                   bool avoid_nodebug,
                   const std::string & target_func,
                   const std::string & avoid_regex,
                   const std::string & step_through_regex) override;

    /// Performs "step over" action on target. Can be called only in
    /// 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    void step_over(unsigned long thread_id,
                   const std::string & step_through_regex) override;

    /// Performs "step out" action on target. Can be called only in
    /// 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    void step_out(unsigned long thread_id,
                  bool avoid_nodebug,
                  const std::string & step_through_regex) override;

    /// Performs "step instrucion into" action on target. Can be called
    /// only in 'stpped state. After calling, the state of target
    /// becomes 'running'.
    void step_inst_into(unsigned long thread_id) override;

    /// Performs "step instrucion over" action on target. Can be called
    /// only in 'stpped state. After calling, the state of target
    /// becomes 'running'.
    void step_inst_over(unsigned long thread_id) override;

    /// Performs "step until" action on target. Can be called only in
    /// 'stopped' state. After calling, the state of target becomes
    /// 'runnig'. Throws exception on error.
    void step_until(unsigned long thread_id, unsigned int line) override;



    //////////////////////////////////////////////////////////////////////
    // breakpoints

    /// Type of breakpoint type
    using breakpoint_type = dbg::breakpoint_type;

    /// Type of breakpoint number
    using breakpoint_num = dbg::breakpoint_num;

    /// Type of breakpoint id
    using breakpoint_id = dbg::breakpoint_id;

    /// Adds breakpoint with specified source position
    void add_breakpoint(const dbg::source_position_info & pos,
                        bool is_enabled,
                        const std::string & condition,
                        const code_breakpoint_handler & handl) override;

    /// Adds breakpoint at function with specified name
    void add_breakpoint(const std::string & func_name,
                        bool is_enabled,
                        const std::string & condition,
                        const code_breakpoint_handler & handl) override;

    /// Adds breakpoint at specified address
    void add_breakpoint(std::uint64_t addr,
                        bool is_enabled,
                        const std::string & condition,
                        const code_breakpoint_handler & handl) override;

    /// Removes breakpoint with specified id
    void remove_breakpoint(const breakpoint_id & id) override;

    /// Enables breakpoint with specified id
    void enable_breakpoint(const breakpoint_id & id) override;

    /// Disables breakpoint with specified id
    void disable_breakpoint(const breakpoint_id & id) override;

    /// Sets condition for breakpoint with specified id
    void set_breakpoint_condition(const breakpoint_id & id, const std::string & cond) override;

    /// Sets ignore count for breakpoint with specified id
    void set_breakpoint_ignore_count(const breakpoint_id & id, unsigned int cnt) override;

    /// Adds watchpoint for variable with specified name
    void add_var_watchpoint(const std::string & name,
                            bool read,
                            bool write,
                            size_t size,
                            const watchpoint_handler & handl) override;

    /// Adds watchpoint for address specified by expression
    void add_expr_watchpoint(const std::string & epxr,
                             bool read,
                             bool write,
                             size_t size,
                             const watchpoint_handler & handl) override;


    //////////////////////////////////////////////////////////////////////
    // Threads

    /// Sets current stack frame
    void set_current_stack_frame(std::size_t index) override;

    /// Sets current thread
    void set_current_thread(std::size_t index) override;

    /// Fetches all stack frames for current thread
    void fetch_all_frames(const thread_handler & handl) override;


    //////////////////////////////////////////////////////////////////////
    // Watch lists

    /// Returns reference to watch list implementation for local variables
    dbg::watch_list_impl & locals_list() override;

    /// Returns reference to custom watch list implementation
    dbg::custom_watch_list_impl & watch_list() override;

    /// Returns reference to registers list implementation
    dbg::watch_list_impl & registers_list() override;


    //////////////////////////////////////////////////////////////////////
    // Input / output

    /// Sends data to stdin of process being debugged
    void send_stdin(const std::string & data) override;


    //////////////////////////////////////////////////////////////////////
    // Memory

    /// Reads memory from target
    void read_memory(uint64_t addr,
                     size_t size,
                     const async::result_handler<std::vector<char>> & h) override;


    //////////////////////////////////////////////////////////////////////
    // Helper functions

    /// Reads list of all modules from core target and converts it to
    /// list of module infos
    static std::shared_ptr<std::vector<dbg::module_info>> read_all_modules(const core::target & targ);

private:
    /// Changes state and emits signal
    void set_state(state_t st,
                   const dbg::process_context * ctx = nullptr,
                   int exit_code = 0);

    /// Extracts process context from core and sets state to stopped
    void process_stopped();

    /// Called in core thread when core target state changed
    void on_core_state_changed(core::target::state_t st);

    /// Called in core thread when new modules loaded
    void on_core_modules_loaded(const std::vector<core::module> & mods);

    /// Called in core thread when modules unloaded
    void on_core_modules_unloaded(const std::vector<core::module> & mods);

    /// Auto selects current frame after stopping
    void auto_select_current_frame();

    typedef std::function<void (core::target &, unsigned long id)> thread_action;

    /// Performs run action on thread with specified id
    void perform_thread_run_action(unsigned long id, const thread_action & act);

    core::debugger & dbg_;                          ///< Reference to debugger
    async::event_queue & main_thread_queue_;        ///< Reference to main thread event queue
    async::execution_queue & core_async_queue_;     ///< Reference to core async execution queue
    std::shared_ptr<core::target> targ_;            ///< Pointer to core target
    state_t state_;                                 ///< Target state
    core_locals_list_impl locals_;                  ///< Locals watch list implementation
    core_custom_watch_list_impl watch_;             ///< Custom watch list implementation
    registers_list_impl registers_;                 ///< Registers list implementation

    bool is_launching_ = false;                     ///< Launching flag
    bool is_stopped_after_launching_ = false;       ///< Was target stopped directly after launching

    ///< List of loaded modules, accessed from core thread only
    std::set<std::filesystem::path> mod_list_;

    /// State changed signal
    boost::signals2::signal<state_changed_func> state_changed_signal_;

    /// Connection to core state changed signal
    boost::signals2::scoped_connection core_state_changed_con_;

    /// Connection to core modules changed signal
    scoped_signal_connection core_modules_changed_con_;

    /// Connection to core modules loaded signal
    scoped_signal_connection core_modules_loaded_con_;

    /// Connection to core modules unloaded signal
    scoped_signal_connection core_modules_unloaded_con_;

    /// Connection to stdout signal
    core::scoped_signal_connection core_stdout_con_;

    /// Connection to stderr signal
    core::scoped_signal_connection core_stderr_con_;
};


}


