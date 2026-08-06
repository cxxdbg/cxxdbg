// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "debugger_state.hpp"
#include "debugger_terminal.hpp"
#include "debugger_ui.hpp"
#include "launch_options.hpp"
#include "source_model.hpp"
#include "term_settings.hpp"
#include "cxxdbg/async/async.hpp"
#include "cxxdbg/app/document_list.hpp"
#include "cxxdbg/app/document_navigator.hpp"
#include "cxxdbg/app/signals.hpp"
#include "cxxdbg/dbg/core/regex_lists.hpp"

#include <ranges.hpp>
#include <filesystem>
#include <functional>
#include <boost/logic/tribool.hpp>
#include <boost/signals2.hpp>


/// \file application.hpp
/// Contains definition of application calss.


namespace cxxdbg::dbg {

namespace cli {
    class app_command_interpreter;
}

class breakpoint_list;
class composite_source_tree_model;
class custom_watch_list;
class dbg_cli_processor;
class dbg_terminal;
class debugger;
class debug_settings;
class platform_connection_options;
class progress_dialog;
class source_tree_model;
class target;
class thread_list;


/// \class application
/// Represents stand alone CXXDBG application, contains application state and event signals
class application: virtual public debugger_ui, public document_navigator {
    /// Type of error handler function
    typedef void error_handler_type(const std::string &);

    /// Type of error handler
    typedef std::function<error_handler_type> error_handler;

    /// Type of current thread changed function
    typedef void current_thread_changed_func();

    /// Type of current thread changed handler
    typedef std::function<current_thread_changed_func> current_thread_changed_handler;

    /// Type of current stack frame changed function
    typedef void current_stack_frame_changed_func();

    /// Type of current stack frame changed handler
    typedef std::function<current_stack_frame_changed_func> current_stack_frame_changed_handler;

    /// Type of async result handler
    typedef std::function<void (const async::result<> &)> async_result_handler;

public:
    using state_t = debugger_state;

    /// Constructor, makes application with specified reference to settings storage,
    /// reference to main thread queue and pointer to debugger implementation
    application(settings_storage & sett,
                async::event_queue & main_thrd_queue,
                const debugger_impl_sp & dbg_impl,
                document_list_ui & doc_ui);

    /// Destructor, terminates application
    ~application();

    /// Returns application state
    state_t state() const;

    /// Returns reference to debugger instance
    debugger & dbg() { return *dbg_; }

    /// Returns const reference to debugger instance
    const debugger & dbg() const { return *dbg_; }

    /// Returns true if there is process being debugged. Returns true
    /// if application state is running or stopped
    bool has_process() const;

    /// Processes command line options
    void process_command_line(const command_line & cmd_line);

    /// Executes command
    void exec_cmd(const std::string & cmd,
                  const std::function<void (const std::string &)> & handler);

    /// Executes vector of commands
    void exec_cmds(const std::vector<std::string> & cmds,
                   const std::function<void (const std::string &)> & handler);

    /// Connects to platform. Dislays progress dialog and blocks execution until
    /// Connection process is finished. Throws exception on error.
    /// Returns false if connection was cancelled by user.
    /// Performs the following:
    /// 1. Asks confirmation to close loaded/started executables
    /// 2. Disconnects from currently connected platofrm
    /// 3. Connects to platform
    bool ask_and_connect(const platform_connection_options & opts);

    /// Disconnects to platform. Returns true if platform was disconnected and set to host.
    /// Performs the following:
    /// 1. Asks confirmation to closed loaded/started executables
    /// 2. Disconnects from platform
    bool ask_and_disconnect();

    /// Loads target with specified path to executable and launch and platform options.
    /// Can be called in any state. Blocks execution and displays progress dialog
    /// unit all operations are finished. Returns false if user cancelled operation.
    /// Throws exception on error. Performs the following steps.
    /// 1. Asks confirmation to close loaded/started executables
    /// 2. Connects to platform if specified in platform options
    /// 3. Loads exectuable
    bool ask_and_load_target(const std::filesystem::path & exe_path,
                             const launch_options & def_l_opts,
                             const platform_connection_options & plat_opts);

    /// Loads target with specified executable path and launch options.
    /// Can be called only in 'initial' state. Waits until target is loaded
    /// After calling, the state of application becomes 'loaded' on 'initial'
    /// in case of error
    void load_target(const std::filesystem::path & exe_path,
                     const launch_options & def_lopts);

    /// Loads target with specified executable path and launch options.
    /// Can be called only in 'initial' state. Waits until target is loaded
    /// After calling, the state of application becomes 'loaded' on 'initial'
    /// in case of error
    void load_target(const std::filesystem::path & exe_path,
                     const std::filesystem::path & work_dir,
                     const std::vector<std::string> & cmd_args);

    /// Starts attaching debugger to process with specified ID or name
    /// and waits until process is attached.
    /// Can be called only in initial or loaded state.
    /// After calling, the state of application becomes
    /// 'stopped' or 'initial' (in case of error)
    void do_attach(const std::variant<unsigned long, std::string> & targ);

    /// Displays dialog for attaching to process and attaches it. Returns ID of attached process.
    /// Returns 0 if operation was cancelled.
    virtual unsigned long ask_pid_and_attach() = 0;

    /// Checks if process is running. If yes then asks user to confirm
    /// termination. After that,
    /// starts attaching to process with specified ID, and waits
    /// until process is attached. Returns false if user cancels attacing.
    /// Throws exception on error
    bool ask_and_attach(const std::variant<unsigned long, std::string> & targ,
                        const platform_connection_options & plat_opts);

    /// Starts closing target. Can be called only in 'loaded state'.
    /// After calling, the state of application becomes 'unloading'.
    void close_target();

    /// Starts closing target and waits until it is closed. Can
    /// be called in any state except initial. After calling, the state
    /// of application becomes 'initial'.
    /// Asks user about terminating process if it is running.
    /// Returns true if executable was closed
    bool close();

    /// Checks if executable is opened. If yes then shows
    /// confirmation dialog about closing it. Returns true
    /// if executable was closed
    bool ask_and_close();

    /// Starts launching loaded target with specified launch options.
    /// After launching is started, displays launching progress and waits
    /// unitl launching is finished. Returns ID of launched process.
    /// Throws exception on error. Can be called only in 'loaded' state.
    /// After calling, the state of application becomes 'running'
    /// or 'loaded' dependinc on success of launching.
    unsigned long launch(const launch_options & lopts, bool save_default);

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
    void do_terminate();

    /// Stops executable if running and waits until stopped.
    /// After that, starts termination and waits until terminated.
    /// Can be called only in 'running' or 'stopped' state.
    /// After call, the state becomes 'loaded'.
    void terminate();

    /// Checks if executable is running. If yes then asks user for
    /// confirmation, stops and terminates process, and waits unit
    /// state becomes 'loaded'. Returns false if user canceled
    /// termination of executable. Can be called in 'initial',
    /// 'loaded, 'stopped', and 'running' state.
    bool ask_terminate();

    /// Starts detaching from current target. Can be called only
    /// in 'stopped' state. After calling, the state of application
    /// becomes 'detaching'.
    void do_detach();

    /// Stops target if running, starts detaching and waits until
    /// detaching is finished. Can be called only in 'stopped' or
    /// 'running' state.
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
    void do_step_until(unsigned int line);

    /// Asks line number and performs "step until" action on current target.
    /// Can be called only in 'stopped' state. After calling, the state of
    /// application becomes 'running'.
    void step_until();

    /// Connects to generic event signal
    signal_connection connect_on_event(const std::function<void (const std::string &)> &handler);

    /// Returns true if target is loaded
    bool has_target() const;

    /// Returns reference to current target.
    target & current_target();

    /// Returns const reference to current target
    const target & current_target() const;

    /// Returns exetuable path for current target
    std::string exe_path();

    /// Returns default launch options
    launch_options def_launch_opts() const;


    //////////////////////////////////////////////////
    // Source related functions

    /// Type of sources changed signal handler function
    typedef void sources_changed_func();

    /// Type of sources changed signal handler
    typedef std::function<sources_changed_func> sources_changed_handler;

    /// Returns const reference to source trees model
    const composite_source_tree_model & trees_model() const;


    /// Returns reference to source_tree data
    source_tree & trees();

    /// Returns const reference to source_tree data
    const source_tree & trees() const;

    /// Returns reference to source model
    source_model & sources();

    /// Returns const reference to source model
    const source_model & sources() const;

    /// Connects to sources changed signal
    signal_connection connect_sources_changed(const sources_changed_handler & handl);

    /// Returns true if position in source code can be displayed
    bool can_show_pos(const source_position & pos);

    /// Shows position in source code
    void show_pos(const source_position & pos);

    /// Shows position in source code
    void show_pos(const std::filesystem::path & path, size_t line) override;

    /// Returns reference to list of opened documents
    document_list & documents() { return documents_; }

    /// Returns const reference to list of opened documents
    const document_list & documents() const { return documents_; }

    /// Returns position of main function
    auto main_pos() const {
        return main_pos_;
    }


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

    /// Fetches all stack frames for current thread
    void fetch_all_stack_frames();

    /// Connects to state_changed signal
    signal_connection connect_state_changed(const std::function<void()> & handler);

    /// Connects to current thread changed signal
    signal_connection connect_current_thread_changed(const current_thread_changed_handler & handl);

    /// Connects to current stack frame changed signal
    signal_connection connect_current_stack_frame_changed(const current_stack_frame_changed_handler & handl);


    //////////////////////////////////////////////////////////////////////
    // Breakpoints

    /// Returns reference to list of breakpoints in target
    breakpoint_list & breakpoints();

    /// Returns reference to const list of breakpoints in target
    const breakpoint_list & breakpoints() const;


    //////////////////////////////////////////////////////////////////////
    // Watch list

    /// Returns reference to locals watch list
    watch_list & locals();

    /// Returns reference to custom watch list
    custom_watch_list & watch();


    //////////////////////////////////////////////////////////////////////
    // Settings

    /// Returns current terminal settings
    const term_settings term_sett() const;

    /// Sets terminal settings
    void set_term_sett(const term_settings & sett);
    
    /// Returns debug settings
    const debug_settings & debug_sett() const;
    
    /// Sets debug settings
    void set_debug_setttings(const debug_settings & sett);


    //////////////////////////////////////////////////////////////////////
    // Command interpreter

    /// Returns reference to command interpreter
    cli::app_command_interpreter & interp();

    /// Returns reference to debugger terminal
    debugger_terminal & dbg_term();


    //////////////////////////////////////////////////////////////////////
    // User interface functions

    /// Notifies user about starting loading module
    void notify_module_load_start(const std::filesystem::path & mod) override;

    /// Notifies user about finishing loading module
    void notify_module_load_finish() override;

    /// Returns status text depending on current application state
    std::wstring status_text() const;


    //////////////////////////////////////////////////////////////////////
    // Executable terminal

    /// Returns reference to executable terminal for current target
    dbg_terminal & exe_term();


private:
    /// Shows temporary status message
    virtual void show_status_message(const std::string & msg) = 0;

    /// Clears temporary status message
    virtual void clear_status_message() = 0;

    /// Shows loading target progress dialog
    virtual std::shared_ptr<progress_dialog>
    make_loading_progress(const std::filesystem::path & exe_path) = 0;

    ///  user about current running process termination. Returns true if user
    /// clicks Yes
    virtual bool ask_terminate_confirmation() = 0;

    /// Asks user about unloading current loaded executable. Returns true
    /// if user selects Yes
    virtual bool ask_close_confirmation() = 0;

    /// Makes progress dialog with specified title, satus text and range of values
    virtual std::shared_ptr<progress_dialog> make_progress_dialog(const std::string & title,
                                                                  const std::string & status,
                                                                  int min = 0,
                                                                  int max = 0) = 0;

    /// Makes launching progress dialog
    virtual std::shared_ptr<progress_dialog> make_launching_progress() = 0;

    /// Makes stopping progress dialog
    virtual std::shared_ptr<progress_dialog> make_stopping_progress() = 0;

    /// Makes terminating progress dialog
    virtual std::shared_ptr<progress_dialog> make_terminating_progress(bool do_detach) = 0;

    /// Makes detaching progress dialog
    virtual std::shared_ptr<progress_dialog> make_detaching_progress() = 0;

    /// Makes attaching progress dialog
    virtual std::shared_ptr<progress_dialog> make_attaching_progress(const std::string & targ) = 0;

    /// Makes closing executable progress dialog
    virtual std::shared_ptr<progress_dialog> make_closing_progress() = 0;

    /// Displays progress dialog while state of application is equal to specified
    void display_progress_while(const std::shared_ptr<progress_dialog> & dlg, state_t st);

    /// Called after current target changed
    void on_target_changed(const async::result<std::shared_ptr<target>> & res,
                           const std::vector<module_info> & mods,
                           const source_position & main_pos);

    /// Called when new modules loaded into current target
    void on_modules_loaded(const std::vector<module_info> & mods);


    settings_storage & settings_;                               ///< Settings storage
    source_model sources_;                                      ///< Source model
    document_list documents_;                                   ///< List of opened documents
    std::unique_ptr<source_tree> src_tree_;                     ///< Source tree
    std::unique_ptr<composite_source_tree_model> src_tree_mdl_; ///< Source trees model
    std::unique_ptr<debugger> dbg_;                             ///< Debugger intance
    launch_options def_launch_opts_;                            ///< Default launch options

    std::shared_ptr<dbg_cli_processor> cli_proc_;               ///< Command processor
    std::shared_ptr<cli::app_command_interpreter> interp_;      ///< Command interpreter
    debugger_terminal term_;                                    ///< Debugger terminal

    boost::signals2::signal<void()> state_changed_signal_;      ///< State changed signal

    source_position main_pos_;                                  ///< Posiition of main function


    // Sources signals

    /// Sources changed signal
    boost::signals2::signal<sources_changed_func> sources_changed_signal_;


    // Threads signals

    /// Connection to target current thread changed signal
    boost::signals2::scoped_connection targ_current_thread_changed_con_;

    /// Connection to target current stack frame changed signal
    scoped_signal_connection targ_current_stack_frame_changed_con_;
};


}


