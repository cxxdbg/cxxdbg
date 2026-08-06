// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file debugger.hpp
/// Contains definition of debugger class.

#pragma once

#include "debug_settings.hpp"
#include "debugger_state.hpp"
#include "term_settings.hpp"

#include "cxxdbg/app/signals.hpp"
#include "cxxdbg/async/async.hpp"

#include <filesystem>
#include <functional>
#include <variant>
#include <boost/logic/tribool.hpp>


namespace cxxdbg {
    class settings_storage;
}


namespace dbgfmt {
    class format_options;
}


namespace cxxdbg::dbg {

class breakpoint_list;
class breakpoints_view_model;
class call_stack_model;
class custom_watch_list;
class dbg_terminal;
class debug_settings;
class debugger_impl;
class debugger_ui;
class launch_options;
class module_info;
class platform;
class platform_connection_options;
class platform_list;
class source_file;
class source_model;
class source_position;
class stack_frame;
class target;
class term_settings;
class thread;
class thread_list;
class watch_list;


/// \class debugger
/// A debugger
class debugger {
public:
    /// Debugger state
    using state_t = debugger_state;

    /// Type of execute result handler
    using execute_result_handler = std::function<void (const std::string &)>;

    /// Type of event handler
    using event_handler = std::function<void (const std::string &)>;

    /// Connect to platform handler
    using connect_to_platform_handler = async::result_handler<>;

    /// Constructor, makes debugger with specified reference to
    /// settings storage,
    /// main thread queue, reference to source model
    /// and shared pointer to debbuger implementation
    debugger(settings_storage & sett,
             async::event_queue & main_thrd_queue,
             source_model & src_model,
             const std::shared_ptr<debugger_impl> & impl,
             debugger_ui & ui);

    /// Destructor, destroys object
    ~debugger();

    /// Returns reference to source model passed in constructor
    source_model & src_mdl() { return src_model_; }

    /// Returns debugger state
    state_t state() const { return state_; }

    /// Returns state name for specified state value
    static const std::string & state_name(state_t st);

    /// Returns name of current state
    const std::string & state_name() const;

    /// The signal is emitted when debugger state changes
    CXXDBG_DEFINE_SIGNALX(state_changed, void())

    /// Executes command with specified text
    void execute(const std::string & cmd, const execute_result_handler &handler);

    /// Executes multiple commands
    void exec_cmds(const std::vector<std::string> & cmds, const execute_result_handler & handler);

    /// Sets settings in debugger
    void set_settings_value(const std::string & name,
                            const std::string & val,
                            const async::result_handler<> & handler = {});

    /// Sets list settings in debugger
    void set_settings_list_value(const std::string & name,
                                 const std::vector<std::string> & values,
                                 const async::result_handler<> & handler = {});

    /// Returns reference to executable terminal
    dbg_terminal & exe_term() { return *exe_term_; }

    /// Returns const reference to debug settings
    const debug_settings & debug_sett() const { return debug_sett_; }

    /// Sets new debug settings and saves it to settings storage
    void set_debug_sett(const debug_settings & sett);

    /// Returns const reference to terminal settings
    const term_settings & term_sett() const { return term_sett_; }

    /// Sets terminal settings and saves it to setting storage
    void set_term_sett(const term_settings & sett);

    /// Returns status text depending on current debugger state
    std::wstring status_text() const;

    /// Returns reference to platform list object
    const platform_list & platforms() const { return *platforms_; }

    /// Slected platform (not connected!). Used onlu in command line interface
    const platform * selected_platform() const { return sel_platform_; }

    /// Sets selected platform
    void set_selected_platform(const platform * p) { sel_platform_ = p; }

    /// Returns platform to which debugger is connected now
    const platform * connected_platform() const { return platform_; }

    /// Connects to platform
    void connect_to_platform(const platform_connection_options & opts,
                             const connect_to_platform_handler & handler = {});

    /// Disconnects from platform
    void disconnect_from_platform();

    /// The signal is emitted when connected platform changed
    signal<void()> connected_platform_changed;


    //////////////////////////////////////////////////
    // Target management

    /// Starts loading target. Can be called only in 'initial' state.
    /// After call, the state of debugger becomes 'loading'.
    void load_target(const std::filesystem::path & exe_path,
                     const async::result_handler<> & handler);

    /// Starts attaching to process with specified ID or name. Can be called in 'initial'
    /// or 'loaded' state. If target is not loaded then state after call becomes
    /// 'loading' and then 'launching'. If target is already loaded then state
    /// after call becomes 'launching'.
    void attach(const std::variant<unsigned long, std::string> & targ,
                const async::result_handler<> & handler);

    /// Returns true if debugger has loaded target
    bool has_target() const;

    /// Returns true if there is process being debugged. Returns true
    /// if application state is running or stopped
    bool has_process() const;

    /// Returns reference to current target. Debugger must have current target
    target & current_target();

    /// Returns const reference to current target. Debugger must have current target
    const target & current_target() const;

    /// Sets current target
    /// TODO: make private
    void set_target(const std::shared_ptr<target> & t) {
        targ_ = t;
    }

    /// Starts closing target. Can be called only in 'loaded' state.
    /// After calling, the state of application becomes 'unloading'.
    void close_target(const async::result_handler<void> & handler);

    /// The signal is emitted after current target changed
    CXXDBG_DEFINE_SIGNALX(target_changed, void(const async::result<std::shared_ptr<target>> & res,
                                             const std::vector<module_info> & mods,
                                             const source_position & main_pos))


    //////////////////////////////////////////////////
    // Process management

    /// Starts launching loaded target with specified launch options.
    /// Can be called only in 'loaded' state.
    /// After call, the state becomes 'launching'.
    void launch(const launch_options & lopts,
                const async::result_handler<unsigned long> & handler);

    /// Continues execution of current target. Can be called only in
    /// 'stopped' state. After calling, the state of application becomes
    /// 'running'.
    void run();

    /// Interrupts execution of current target. Can be called only
    /// in 'stopped' or 'running' state. After calling, the state of
    /// application does not change until target is stopped.
    void stop();

    /// Starts terminating execution of current target. Can be called only
    /// in 'stopped' state. After calling, the state of application
    /// becomes 'terminating'.
    void terminate();

    /// Starts detaching from current target. Can be called only
    /// in 'stopped' state. After calling, the state of application
    /// becomes 'detaching'.
    void detach();

    /// Performs "step into" action on current target. Can be called
    /// only in 'stopped' state. After calling, the state of
    /// application becomes 'running'
    void step_into(boost::tribool avoid_nodebug = boost::indeterminate,
                   const std::string & target = {},
                   const std::string & avoid_regex = {},
                   const std::string & step_through_regex = {});

    /// Performs step into target
    void step_into_call_target(const std::string & target_name);

    /// Performs "step over" action on current target. Can be called
    /// only in 'stopped' state. After calling, the state of
    /// application becomes 'running'
    void step_over(const std::string & step_through_regex = {});

    /// Performs "step out" action on current target. Can be called
    /// only in 'stopped' state. After calling, the state of
    /// application becomes 'running'
    void step_out(boost::tribool avoid_nodebug = boost::indeterminate,
                  const std::string & step_through_regex = {});

    /// Performs "step instruction into" action on current target. Can be
    /// called only in 'stopped' state. After calling, the state of
    /// application becomes 'running'.
    void step_inst_into();

    /// Performs "step instruction over" action on current target. Can be
    /// called only in 'stopped' state. After calling, the state of
    /// application becomes 'running'.
    void step_inst_over();

    /// Performs "step until" action with specified line number on current
    /// target. Can be called only in 'stopped' state. After calling, the
    /// state of application becomes 'running'.
    void step_until(unsigned int line);

    /// Asks user to enter line number and performs "step until" action with
    /// entered line number on current target. Can be called only in 'stopped' state.
    /// After calling, the state of application becomes 'running' or 'stopped'
    /// if user cancels action
    void ask_line_number_and_step_until();

    /// Returns reference to list of call targets at current position.
    /// TODO: make const
    auto & call_targets() { return current_call_targets_; }

    /// The signal is emitted when list of current call targets changed
    CXXDBG_DEFINE_SIGNALX(call_targets_changed, void ())

    /// Process exit signal. The signal is emitted after process exit
    CXXDBG_DEFINE_SIGNALX(process_exited, void (int))


    //////////////////////////////////////////////////////////////////////
    // Breakpoints

    /// Returns reference to list of breakpoints
    breakpoint_list & breakpoints() { return *breakpoints_; }

    /// Returns const reference to list of breakpoints
    const breakpoint_list & breakpoints() const { return *breakpoints_; }

    /// Returns reference to breakpoint list tree view model
    auto & breakpoints_model() { return *breakpoints_model_; }


    //////////////////////////////////////////////////

    /// Connects to event signal
    signal_connection connect_on_event(const event_handler & h);

    /// Notifies user about starting loading module
    void notify_module_load_start(const std::filesystem::path & mod);

    /// Notifies user about starting loading module
    void notify_module_load_finish();

    /// Displays info message in application
    void show_info_message(const std::wstring & msg);


    //////////////////////////////////////////////////
    // Threads

    /// Returns reference to list of threads for current target.
    /// Can be called only in stopped state
    const thread_list & threads() const;

    /// Returns const pointer to current thread or null if no current thread.
    const thread * current_thread() const;

    /// Sets specified thread as current thread. Can be called only
    /// in stopped state.
    void set_current_thread(const thread * thrd);

    /// Returns const pointer to current stack frame, or null if no current stack frame.
    const stack_frame * current_frame() const;

    /// Sets current stack frame. Can be called only in stopped state
    void set_current_frame(const stack_frame * frame);

    /// Returns reference to call stack model
    call_stack_model & call_stack() { return *cs_mdl_; }

    /// Fetches all stack frames for current thread
    void fetch_all_stack_frames();

    /// The signal is emitted after current thread changed
    CXXDBG_DEFINE_SIGNALX(current_thread_changed, void())

    /// The signal is emitted after current frame changed
    CXXDBG_DEFINE_SIGNALX(current_frame_changed, void())

    /// Returns map of lines to threads that have current positions in lines
    /// for specified source file
    std::multimap<unsigned int, const thread *>
    source_threads(const source_file * src) const;

    /// Returns list of threads that have current position at specified line of source file
    std::list<const thread *> line_threads(const source_file * src, unsigned int line) const;

    /// Returns current position of current thread if it's available
    /// or invalid position if not.
    source_position curr_thread_pos() const;

    /// Returns position of current frame in source code or invalid position
    /// if it's not available
    source_position curr_frame_pos() const;


    //////////////////////////////////////////////////
    // Watch lists

    /// Returns reference to locals watch list
    watch_list & locals() { return *locals_; }

    /// Returns reference to custom watch list
    custom_watch_list & watch() { return *watch_; }

    // Returns reference to registers watch list
    watch_list & registers() { return *registers_; }

    /// Returns current format options
    const dbgfmt::format_options & fmt_opts() const;

    /// Sets current format options and saves them to settings
    void set_fmt_opts(const dbgfmt::format_options & opts);

    /// Returns current display hex format option
    bool fmt_hex() const;

    /// Sets current display hex format option
    void set_fmt_hex(bool val);

    /// Returns current display pointer addresses format option
    bool fmt_ptr_addr() const;

    /// Sets current display pointer addresses format option
    void set_fmt_ptr_addr(bool val);

    /// Returns current display record addresses format option
    bool fmt_rec_addr() const;

    /// Sets current display record addresses format option
    void set_fmt_rec_addr(bool val);

    /// The signal is emitted when formatting options changed
    CXXDBG_DEFINE_SIGNALX(fmt_opts_changed, void())


    //////////////////////////////////////////////////
    // User Interface

    /// Returns description of line in source code that should be displayed in
    /// code viewer
    std::string line_description(const source_position & pos, bool show_threads) const;


private:
    /// Initializes terminal settings
    void init_term_settings();

    /// Called when loading target / attaching to target is complete.
    /// Sets target and setups listener for target signals
    void on_target_load_attach_complete(const async::result<std::shared_ptr<target>> & res,
                                        const std::vector<module_info> & mods,
                                        const source_position & main_pos);

private:
    /// Sets debugger state
    void set_state(state_t s);

    /// Called when target state was changed
    void on_target_state_changed();


    settings_storage & settings_;               ///< Settings storage
    debug_settings debug_sett_;                 ///< Debug settings
    term_settings term_sett_;                   ///< Terminal settings

    std::unique_ptr<platform_list> platforms_;  ///< List of supported platforms
    const platform * sel_platform_ = nullptr;   ///< Selected platform (used only in CLI)
    const platform * platform_ = nullptr;       ///< Connected platform
    state_t state_;                             ///< Debugger state
    async::event_queue & main_thread_queue_;    ///< Reference to main thread event queue
    source_model & src_model_;                  ///< Reference to source model
    std::shared_ptr<debugger_impl> impl_;       ///< Pointer to debugger implementaton
    debugger_ui & ui_;                          ///< Reference to debugger UI implementation
    std::shared_ptr<target> targ_;              ///< Current target

    /// List of breakpoints
    std::unique_ptr<breakpoint_list> breakpoints_;

    /// Call stack model
    std::unique_ptr<call_stack_model> cs_mdl_;

    /// Breakpoints view model
    std::unique_ptr<breakpoints_view_model> breakpoints_model_;

    std::unique_ptr<dbg_terminal> exe_term_;    ///< Executable terminal

    /// Locals watch list
    std::unique_ptr<watch_list> locals_;

    /// Custom watch list
    std::unique_ptr<custom_watch_list> watch_;

    /// Registers watch list
    std::unique_ptr<watch_list> registers_;

    ///< List of call targets for step into function menu
    std::vector<std::string> current_call_targets_;

    bool target_was_running_ = false;           ///< True if target was in running state previously
};


}


