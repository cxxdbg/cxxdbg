// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file target.hpp
/// Contains definition of target class.

#pragma once

#include "source_position.hpp"
#include "target_impl.hpp"
#include "target_state.hpp"
#include "cxxdbg/async/async.hpp"
#include "cxxdbg/exec_stub/exec_stub_server.hpp"

#include <ranges.hpp>
#include <thread>
#include <vector>
#include <boost/signals2.hpp>


namespace cxxdbg::dbg {

class breakpoint_list;
class launch_options;
class module;
class source_model;
class stop_reason;
class term_settings;
class thread_list;
class watch_list_impl;

typedef std::shared_ptr<module> module_sp;


/// \class target
/// Represents single debugging target
class target {
    // launch_helper calls clear_threads after temporary stops
    friend class launch_helper;
    friend class breakpoint_list;
    friend class debugger;

public:
    /// Target state
    using state_t = target_state;

    /// Type of connection to signal
    typedef boost::signals2::connection connection;

    /// Type of scoped connection to signal
    typedef boost::signals2::scoped_connection scoped_connection;

    /// Type of state changed handler function
    typedef void state_changed_func();

    /// Type of state changed handler
    typedef std::function<state_changed_func> state_changed_handler;

    /// Type of current thread changed handler function
    typedef void current_thread_changed_func();

    /// Type of current thread changed handler
    typedef std::function<current_thread_changed_func> current_thread_changed_handler;

    /// Type of current stack frame changed handler function
    typedef void current_stack_frame_changed_func();

    /// Type of current stack frame changed handler
    typedef std::function<current_stack_frame_changed_func> current_stack_frame_changed_handler;

    /// Type of call targets list
    typedef std::vector<std::string> call_targets_list;


    /// Constructor, makes target with specified reference to main thread event queue,
    /// pointer to target implementation,
    /// reference to source model, path to executable, and list of target modules
    target(debugger & d,
           async::event_queue & main_thrd_queue,
           const std::shared_ptr<target_impl> & tgt,
           source_model & src_model,
           breakpoint_list & bplist,
           const std::vector<module_info> & mods,
           const std::filesystem::path & epath = std::filesystem::path());

    /// Destructor, destroys target. Target must be destroyed only in
    /// loaded state
    ~target();

    /// Returns path to executable
    std::string exe_path();


    //////////////////////////////////////////////////
    // Debug info

    /// Type of sources changed signal handler function
    typedef void sources_changed_func();

    /// Type of sources changed signal handler
    typedef std::function<sources_changed_func> sources_changed_handler;

    /// Finds functions with specified prefix
    std::vector<const char*> find_funcs(const std::string & prefix) const;

    /// Connects to sources changed signal
    connection connect_sources_changed(const sources_changed_handler & handl);

    const std::vector<std::string> & get_call_targets() const;

    /// The signal is emitted when new modules loaded
    CXXDBG_DEFINE_SIGNALX(modules_loaded, void (const std::vector<module_info> &))


    //////////////////////////////////////////////////
    // Process state

    /// Returns target state
    state_t state() const;

    /// Returns true if target was launched in terminal
    bool launched_in_term() const { return launched_in_term_; }

    /// Returns stop reason for target in stopped state.
    const stop_reason & stop_rsn() const;

    /// Returns last exit code for target
    int exit_code() const;

    /// Starts launching target with specified working directory and
    /// command arguments.. May be called only in loaded
    /// state. After calling, the state of target becomes launching.
    void launch(const launch_options & lopts,
                const async::result_handler<unsigned long> & handler);

    /// Starts launching target in terminal with specified working directory
    /// and command arguments. Calls completion handler after launching
    /// has been started. May be called only in loaded state.
    /// After calling, the state of target becomes launching.
    void launch_terminal(const launch_options & lopts,
                         const term_settings & term_sett,
                         const async::result_handler<unsigned long> & handl);

    /// Starts attaching target to process with specified process id or name.
    /// May be called only in loaded state. After calling the state of
    /// target becomes launching.
    void attach(const std::variant<unsigned long, std::string> & targ,
                const async::result_handler<> & h);

    /// Continue execution of target. May be called in stopped state.
    /// After calling, the state of target becomes running.
    void run();

    /// Starts stopping target process. May be called only in running
    /// or stopped state. After calling, the state of target does not change
    /// before actual stopping of target.
    void stop();

    /// Starts terminating target process. May be called only in stopped
    /// state. After calling, the state of target becomes terminating.
    void terminate();

    /// Starts detaching from target process. May be called only in stopped
    /// state. After calling, the state of target becomes detaching.
    void detach();

    /// Deletes target and makes it invalid. Can be called only in
    /// 'loaded' state. Calls specified handler after deleting target.
    void close(const std::function<void()> & handl);

    /// Performs "step into" action on target. Can be called only in
    /// 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    void step_into(bool avoid_nodebug,
                   const std::string & target_func,
                   const std::string & avoid_regex,
                   const std::string & step_through_regex);

    /// Performs "step over" action on target. Can be called only in
    /// 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    void step_over(const std::string & step_through_regex);

    /// Performs "step out" action on target. Can be called only in
    /// 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    void step_out(bool avoid_nodebug, const std::string & step_through_regex);

    /// Performs "step instruction into" action on target. Can be called
    /// only in 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    void step_inst_into();

    /// Performs "step instruction over" action on target. Can be called
    /// only in 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    void step_inst_over();

    /// Performs "step until" action on target. Can be called only in
    /// 'stopped' state. After calling, the state of target becomes
    /// 'running'.
    void step_until(unsigned int line);

    /// Returns const reference to thread list
    const thread_list & threads() const;

    /// Returns reference to thread list
    thread_list & threads();

    /// Returns const pointer to current thread or null if no current thread.
    const thread * current_thread() const;

    /// Sets current thread. Can be called only in stopped state
    void set_current_thread(const thread * thrd);

    /// Returns const pointer to current stack frame or null if no current stack frame.
    const stack_frame * current_frame() const;

    /// Sets current stack frame. Can be called only in stopped state
    void set_current_frame(const stack_frame * frame);

    /// Fetches all stack frames for current thread
    void fetch_all_stack_frames();

    /// Connects to state changed signal
    boost::signals2::connection connect_state_changed(const state_changed_handler & handl);

    /// Connects to current thread changed signal
    boost::signals2::connection connect_current_thread_changed(
            const current_thread_changed_handler & handl);

    /// Connects to current stack frame changed signal
    connection connect_current_stack_frame_changed(const current_stack_frame_changed_handler & handl);

    /// Returns ID of target process. May be called only in running ir stopped state.
    unsigned long pid();


    //////////////////////////////////////////////////////////////////////
    // Watch list

    /// Returns reference to locals watch list implementation
    watch_list_impl & locals();

    /// Returns reference to custom wtch list implementation
    custom_watch_list_impl & watch();

    /// Returns reference to registers watch list implementation
    watch_list_impl & registers();


    //////////////////////////////////////////////////////////////////////
    // Input / output

    /// Stdout data received signal
    CXXDBG_DEFINE_SIGNALX(stdout_received, void(const std::string &))

    /// Stderr data received signal
    CXXDBG_DEFINE_SIGNALX(stderr_received, void(const std::string &))

    /// Sends data to stdin of process being debugged
    void send_stdin(const std::string & data);


    //////////////////////////////////////////////////////////////////////
    // Memory

    /// Reads memory from target
    void read_memory(uint64_t addr, size_t size, const async::result_handler<std::vector<char>> & h);


    //////////////////////////////////////////////////////////////////////
    // Utilities

    /// Converts source position info to source position resolving source file
    static source_position convert_source_position_info(source_model & src_mdl,
                                                        const source_position_info & pos_info);

    /// Converts code position info to code position resolving source file
    static code_position convert_code_position_info(source_model & src_mdl,
                                                    const code_position_info & pos_info);


private:
    /// Clears all threads. Used in launch_helper for clearing
    /// threads after temporary stops
    void clear_threads();

    /// Type of shared pointer to thread list
    typedef std::shared_ptr<thread_list> thread_list_ptr;

    // noncopyable
    target(const target &) = delete;
    target & operator=(const target &) = delete;

    /// Called when implementation state changed signal received
    // TODO: refactor and move process context and exit code to single parameter
    void on_impl_state_changed(target_impl::state_t st,
                               const process_context * ctx,
                               int exit_code);

    /// Called when launching exec_stub executable is complete, and
    /// PID of exec_stub is received. Starts attaching to process with
    /// specified PID. Calls result handler after attaching has beee
    /// started
    void on_exec_stub_launched(std::istream & child_stderr,
                               unsigned long pid,
                               const async::result_handler<> & handl);

    /// Updates list of target sources
    void add_modules(const std::vector<module_info> & mods);

    /// Returns reference to target implementation
    target_impl & impl() { return *targ_; }


    debugger & dbg_;                        ///< Reference to debugger object
    async::event_queue & main_thread_queue_;///< Main thread event queue
    std::shared_ptr<target_impl> targ_;     ///< Pointer to target implementation
    source_model & src_model_;              ///< Reference to source model
    std::unique_ptr<thread_list> threads_;  ///< Thread list
    breakpoint_list & bplist_;              ///< Reference to breakpoint list

    std::filesystem::path exe_path_;      ///< Path to executable
    unsigned long pid_;                     ///< ID of target process
    std::shared_ptr<stop_reason> stop_rsn_; ///< Stop reason for stopped state
    int exit_code_;                         ///< Returns last exit code for target
    std::vector<module_sp> mods_;           ///< Vector of modules in target
    bool launched_in_term_ = false;         ///< True if target is launched in terminal

    call_targets_list call_targets_;        ///< Current call targets

    /// Exec stub server for interacting with exec_stub and debuggee
    std::unique_ptr<exec_stub::exec_stub_server> exec_stub_srv_;

    /// Thread for waiting for launching exec_stub executabler
    std::shared_ptr<std::thread> launch_exec_stub_thrd_;

    /// Source list changed signal
    boost::signals2::signal<sources_changed_func> sources_changed_signal_;

    /// State changed signal
    boost::signals2::signal<state_changed_func> state_changed_signal_;

    /// Current thread changed signal
    boost::signals2::signal<current_thread_changed_func> current_thread_changed_signal_;

    /// Current stack frame changed signal
    boost::signals2::signal<current_stack_frame_changed_func> current_stack_frame_changed_signal_;

    /// Connection to debugger event signal
    boost::signals2::scoped_connection event_con_;

    /// Connection to implementation state changed signal
    scoped_signal_connection impl_state_changed_con_;

    /// Connection to start module load signal
    scoped_signal_connection impl_start_module_load_con_;

    /// Connection to stop module load signal
    scoped_signal_connection impl_finish_module_load_con_;

    /// Connection to implementation modules loaded signal
    scoped_signal_connection impl_modules_loaded_con_;

    /// Connection to implementation modules unloaded signal
    scoped_signal_connection impl_modules_unloaded_con_;
};


}


