// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file target_impl.hpp
/// Contains definition of target_impl class.

#pragma once

#include "breakpoint_info.hpp"
#include "breakpoint_list_impl.hpp"
#include "forward.hpp"
#include "target_state.hpp"
#include "cxxdbg/app/signals.hpp"
#include "cxxdbg/async/async.hpp"
#include <filesystem>
#include <variant>


namespace cxxdbg::dbg {


class custom_watch_list_impl;
class thread_info;

typedef std::shared_ptr<thread_info> thread_info_sp;


/// \class target_impl
/// Incapsulates implementation of async data fetching from target
class target_impl: virtual public breakpoint_list_impl {
public:
    /// Target state
    using state_t = target_state;

    /// Returns name of state value
    static std::string state_name(state_t st);

    /// Destructor, destroys object
    virtual ~target_impl() {}


    //////////////////////////////////////////////////
    // Modules

    /// Type of vector of module infos
    typedef std::vector<module_info> module_info_vector;

    /// Type of vector of module paths
    typedef std::vector<std::filesystem::path> module_path_vector;

    /// Type of start module load signal handler function
    typedef void start_module_load_func(const std::filesystem::path &);

    /// Type of modules loaded signal handler function
    typedef void modules_loaded_func(const module_info_vector &);

    /// Type of modules unloaded signal handler function
    typedef void modules_unloaded_func(const module_path_vector &);

    /// Type of start module load signal handler
    typedef std::function<start_module_load_func> start_module_load_handler;

    /// Type of modules loaded signal handler
    typedef std::function<modules_loaded_func> modules_loaded_handler;

    /// Type of module unloaded signal handler
    typedef std::function<modules_unloaded_func> modules_unloaded_handler;

    /// The signal is emitted when implementation starts loading module
    CXXDBG_DEFINE_SIGNALX(module_load_started, void(const std::filesystem::path &))

    /// The signal is emitted when implementation finishes loading module
    CXXDBG_DEFINE_SIGNALX(module_load_finished, void())

    /// The signal is emitted when new moduels loaded
    CXXDBG_DEFINE_SIGNALX(modules_loaded, void (const module_info_vector &))

    /// The signal is emitted when modules unloaded
    CXXDBG_DEFINE_SIGNALX(modules_unloaded, void (const module_path_vector &))


    //////////////////////////////////////////////////
    // Target state

    /// Type of state changed handler function
    typedef void state_changed_func(state_t, const process_context *, int exit_code);

    /// Type of state changed handler
    typedef std::function<state_changed_func> state_changed_handler;

    /// Connects to state changed signal
    virtual signal_connection connect_state_changed(const state_changed_handler & handl) = 0;

    /// Returns target implementation state
    virtual state_t state() const = 0;

    /// Starts launching target with specified working directory and
    /// command arguments. May be called only in loaded
    /// state. After calling, the state of target becomes launching.
    virtual void launch(const launch_options & lopts,
                        const async::result_handler<unsigned long> & handler) = 0;

    /// Starts attaching target to process with specified process
    /// id or name. May be called only in loaded state. After calling, the
    /// state of target becomes launching.
    virtual void attach(const std::variant<unsigned long, std::string> & targ,
                        const async::result_handler<> & h) = 0;

    /// Continue execution of target. May be called in stopped state.
    /// After calling, the state of target becomes running.
    virtual void run() = 0;

    /// Starts stopping target process. May be called only in running
    /// or stopped state. After calling, the state of target does not change
    /// before actual stopping of target.
    virtual void stop() = 0;

    /// Starts terminating target process. May be called only in stopped
    /// state. After calling, the state of target becomes terminating.
    virtual void terminate() = 0;

    /// Starts detaching from target process. May be called only in
    /// stopped state. After calling, the state of target becomes detaching.
    virtual void detach() = 0;

    /// Deletes target and makes it invalid. Can be called only in
    /// 'loaded' state. Calls specified handler after deleting target.
    virtual void close(const std::function<void()> & handl) = 0;

    /// Performs "step into" action on target. Can be called only in
    /// 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    virtual void step_into(unsigned long thread_id,
                           bool avoid_nodebug,
                           const std::string & target,
                           const std::string & avoid_regex,
                           const std::string & step_through_regex) = 0;

    /// Performs "step over" action on target. Can be called only in
    /// 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    virtual void step_over(unsigned long thread_id,
                           const std::string & step_through_regex) = 0;

    /// Performs "step out" action on target. Can be called only in
    /// 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    virtual void step_out(unsigned long thread_id,
                          bool avoid_nodebug,
                          const std::string & step_through_regex) = 0;

    /// Performs "step instrucion into" action on target. Can be called
    /// only in 'stpped state. After calling, the state of target
    /// becomes 'running'.
    virtual void step_inst_into(unsigned long thread_id) = 0;

    /// Performs "step instrucion over" action on target. Can be called
    /// only in 'stpped state. After calling, the state of target
    /// becomes 'running'.
    virtual void step_inst_over(unsigned long thread_id) = 0;

    /// Performs "step until" action on target. Can be called only in
    /// 'stopped' state. After calling, the state of target becomes
    /// 'runnig'. Throws exception on error.
    virtual void step_until(unsigned long thread_id, unsigned int line) = 0;


    //////////////////////////////////////////////////////////////////////
    // Threads

    typedef std::function<void (const thread_info_sp &)> thread_handler;

    /// Sets current stack frame
    virtual void set_current_stack_frame(std::size_t index) = 0;

    /// Sets current thread
    virtual void set_current_thread(std::size_t index) = 0;

    /// Fetches all stack frames for current thread
    virtual void fetch_all_frames(const thread_handler & handl) = 0;


    //////////////////////////////////////////////////////////////////////
    // Watch lists

    /// Returns reference to watch list implementation for local variables
    virtual watch_list_impl & locals_list() = 0;

    /// Returns reference to custom watch list implementation
    virtual custom_watch_list_impl & watch_list() = 0;

    /// Returns reference to watch list implementation for registers
    virtual watch_list_impl & registers_list() = 0;


    //////////////////////////////////////////////////////////////////////
    // Input / output

    /// Stdout data received signal
    CXXDBG_DEFINE_SIGNALX(stdout_received, void(const std::string &))

    /// Stderr data received signal
    CXXDBG_DEFINE_SIGNALX(stderr_received, void(const std::string &))

    /// Sends data to stdin of process being debugged
    virtual void send_stdin(const std::string & data) = 0;


    //////////////////////////////////////////////////////////////////////
    // Memory

    /// Reads memory from target
    virtual void read_memory(uint64_t addr,
                             size_t size,
                             const async::result_handler<std::vector<char>> & h) = 0;
};


}


