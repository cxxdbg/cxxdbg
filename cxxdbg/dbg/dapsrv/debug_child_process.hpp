// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file debug_child_process.hpp
/// Contais definition of the debug_child_process class.

#pragma once

#include "asio_event_queue.hpp"
#include "child_process.hpp"
#include "variable_reference.hpp"
#include "cxxdbg/dbg/core/debugger.hpp"
#include "cxxdbg/dbg/core/target.hpp"
#include "dbgfmt/format_options.hpp"
#include <boost/asio/executor.hpp>
#include <regex>


namespace cxxdbg::dbg::dapsrv {


/// Debug process launched for debugging with CXXDBG
class debug_child_process: virtual public child_process {
public:
    using source_breakpoints_map = std::map<std::filesystem::path, std::vector<std::tuple<size_t, dap::source_breakpoint>>>;
    using function_breakpoints_vector = std::vector<std::tuple<size_t, dap::function_breakpoint>>;
    using instruction_breakpoints_vector = std::vector<std::tuple<size_t, dap::instruction_breakpoint>>;

    /// Constructs child process with specified ASIO executor
    debug_child_process(const boost::asio::executor & exec):
        exec_{exec} {}

    /// Prepares child proces to be launched with specified path to executable and arguments
    virtual void prepare_launch(const path_t & exe_path,
                                const path_t & work_dir,
                                const std::vector<std::wstring> & args,
                                const std::map<std::wstring, std::wstring> & env,
                                const json_t & json_parameters,
                                const source_breakpoints_map & source_breakpoints,
                                const function_breakpoints_vector & function_breakpoints,
                                const instruction_breakpoints_vector & instruction_breakpoints,
                                const std::vector<std::wstring> & exception_breakpoint_filters);

    /// Processes configuration done request. Launches process for debugging or resumes
    /// attached process
    void process_configuration_done(const std::function<void (const std::string &)> & comp_handler) override;

    /// Attaches to process with specified ID or name
    void attach(size_t pid,
                const std::wstring & program,
                const json_t & json_parameters,
                const source_breakpoints_map & source_breakpoints,
                const function_breakpoints_vector & function_breakpoints,
                const instruction_breakpoints_vector & instruction_breakpoints,
                const std::vector<std::wstring> & exception_breakpoint_filters);

    /// Processes threads request
    std::vector<dap::thread> threads() override;

    /// Processes stack trace request
    size_t stack_trace(size_t thread_id,
                       size_t first,
                       size_t max_cnt,
                       const dap::stack_frame_format & fmt,
                       std::vector<dap::stack_frame> & res) override;

    /// Processes scopes request
    std::vector<dap::scope> scopes(size_t frame_id) override;

    /// Processes variables request
    std::vector<dap::variable> variables(size_t var_ref,
                                         dap::variable_filter filter,
                                         size_t start,
                                         size_t count,
                                         const dap::value_format & fmt) override;

    /// Processes evaluate request
    dap::evaluate_response evaluate(const std::wstring & expr,
                                    size_t frame_id,
                                    const std::wstring & context,
                                    const dap::value_format & fmt) override;

    /// Processes set breakpoints request. Returns vector of breakpoints set.
    void set_breakpoints(const path_t & src,
                         const std::vector<dap::source_breakpoint> & bpts,
                         const std::vector<size_t> & line_numbers,
                         bool source_modified) override;

    /// Processes set exception breakpoints request
    void set_exception_breakpoints(const std::vector<std::wstring> & filters,
                                   const std::vector<dap::exception_options> & opts) override;

    /// Processes set function breakpoints request
    void set_function_breakpoints(const std::vector<dap::function_breakpoint> & bpts) override;

    /// Processes set instruction breakpoints request
    void set_instruction_breakpoints(const std::vector<dap::instruction_breakpoint> & bpts) override;

    /// Processes continue request from client
    bool process_continue(size_t thread_id) override;

    /// Processes pause request
    void pause(size_t thread_id) override;

    /// Processes next request
    void next(size_t thread_id) override;

    /// Processes next instruction request
    void next_inst(size_t thread_id) override;

    /// Processes step in request
    void step_in(size_t thread_id, size_t target_id) override;

    /// Processes instruction step in request
    void step_inst_in(size_t thread_id) override;
    
    /// Processes step out request
    void step_out(size_t thread_id) override;

    /// Processes disconnect request. Terminates child if needed and emits disconnected signal
    /// when all required operations are done
    void disconnect(bool terminate) override;

    /// Processes terminate request
    void terminate() override;

    /// Disassembles memory
    std::vector<dap::disassembled_instruction>
    disassemble(uint64_t addr, int64_t inst_offset, size_t inst_count, bool resolve_symbols) override;

    /// Reads memory
    dap::read_memory_result read_memory(std::wstring_view mem_ref, int64_t offset, size_t size) override;

protected:
    /// Returns true if process being debugged was attached and not launched
    bool was_attached() const { return attach_pid_ != SIZE_MAX; }

    /// Sets number of initial stopts to skip after target is launched
    void set_num_skip_init_stops(int n) { num_skip_init_stops_ = n; }

    /// Loads target with specified executable path
    void load_target(const path_t & exe_path);

    /// Returns reference to current CXXDBG target. Throws exception
    /// if target does not exist
    core::target & targ();

    /// Prepares target after launching or attaching
    void prepare_target(const source_breakpoints_map & source_breakpoints,
                        const function_breakpoints_vector & function_breakpoints,
                        const instruction_breakpoints_vector & instruction_breakpoints,
                        const std::vector<std::wstring> & exception_breakpoint_filters);

    /// Processes common debug JSON parameters
    void process_common_debug_parameters(const json_t & parameters);

private:
    /// Procesess target state change signal
    void on_target_state_changed(core::target::state_t st);

    /// Prepares target for run. Clears stack frame maps, etc
    void prepare_to_run();

    /// Installs breakpoints into current target
    void install_breakpoints(const source_breakpoints_map & source_breakpoints,
                             const function_breakpoints_vector & function_breakpoints,
                             const instruction_breakpoints_vector & instruction_breakpoints,
                             const std::vector<std::wstring> & exception_breakpoint_filters);

    /// Adds source breakpoint into current target
    core::code_breakpoint * add_source_breakpoint(const std::filesystem::path & src, const dap::source_breakpoint & src_bp);

    /// Adds function breakpoint into current target
    core::code_breakpoint * add_function_breakpoint(const dap::function_breakpoint & func_bp);

    /// Adds instruction breakpoint into current target
    core::code_breakpoint * add_instruction_breakpoint(const dap::instruction_breakpoint & inst_bp);

    /// Adds EH filter breakpoint into current target
    core::code_breakpoint * add_eh_breakpoint(const std::wstring & filter);

    /// Sets exception breakpoints for specified EH filters
    void set_eh_breakpoints(const std::vector<std::wstring> & filters);

    /// Sets current thread by id
    void set_current_thread(size_t thread_id);

    /// Searches for thread with specified ID. Returns unique pointer to found
    /// thread of null pointer if thread not found
    std::unique_ptr<core::thread> try_thread_by_id(size_t id);

    /// Searches for thread with specified ID. Throws exception if thread is not found
    core::thread thread_by_id(size_t id);

    /// Sets up common breakpoint options
    void setup_breakpoint(core::code_breakpoint * bp,
                          const std::wstring & condition,
                          const std::wstring & hit_condition,
                          const std::wstring & log_message);

    /// Called when new modules loaded
    void on_modules_loaded(const std::vector<core::module> & mods);


    /// Should functions with no debug info be avoided while performing stepping?
    bool avoid_nodebug_ = true;

    /// Regex for matching functions that should be avoided while perforing stepping
    std::string avoid_regex_;

    /// Regex for matching functions that should be stepped through while performing step into
    std::string step_through_regex_;

    /// Regex for grouping frames
    std::regex group_regex_;

    boost::asio::executor exec_;            ///< ASIO executor for executing code in work thread
    asio_event_queue eq_{exec_};            ///< Event queue wrapper for ASIO executor
    core::debugger dbg_{eq_};               ///< Debugger instance
    std::shared_ptr<core::target> targ_;    ///< Current target
    dbgfmt::format_options fmt_opts_;          ///< Format options

    path_t work_dir_;                           ///< Working directory
    std::vector<std::wstring> args_;            ///< Launch arguments
    std::map<std::wstring, std::wstring> env_;  ///< Launch environment

    bool is_launching_ = false;                 ///< Launching flag
    bool is_stopped_after_launching_ = false;   ///< Was target stopped directly after launching

    int num_skip_init_stops_ = 0;           ///< Rest count of stops to be skipped during launch

    /// ID of process to attach to
    size_t attach_pid_ = SIZE_MAX;

    bool terminate_requested_ = false;      ///< Was terminate requested?
    bool disconnect_requested_ = false;     ///< Was disconnect requested?

    /// Exception thrown breakpoint
    core::code_breakpoint * eh_thrown_bp_ = nullptr;

    /// Exception caught breakpoint
    core::code_breakpoint * eh_caught_bp_ = nullptr;

    /// Variable reference table
    std::unique_ptr<variable_reference_table> vars_;

    /// Thread ID requested in pause
    size_t requested_pause_thread_id_ = SIZE_MAX;

    /// Map from stack frames to IDs
    std::map<core::stack_frame, size_t, stack_frame_compare> frame_ids_;

    std::map<size_t, core::stack_frame> frames_;        ///< Map of stack frames
    size_t next_stack_frame_id_ = 1;                    ///< Next stack frame ID
};


}
