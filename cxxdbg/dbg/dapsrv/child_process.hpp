// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file child_process.hpp
/// Contains definition of the child_process class.

#pragma once

#include <cxxdap/protocol_json.hpp>
#include <filesystem>
#include <boost/signals2/signal.hpp>


namespace cxxdbg::dbg::dapsrv {


namespace dap = ::cxxdap;


/// Represents abstract child process launched from dap server
/// (for debugging or without debugging)
/// Handles child specific DAP server requests and generates signals
class child_process {
public:
    using json_t = dap::json_t;
    using path_t = std::filesystem::path;
    template <typename Signature> using signal_t = boost::signals2::signal<Signature>;

    /// Default virtual destructor
    virtual ~child_process() = default;

    /// Processes configuration done request. Launches process for debugging or resumes
    /// attached process
    virtual void process_configuration_done(const std::function<void (const std::string &)> & comp_handler) = 0;

    /// Processes disconnect request. Terminates child if needed and emits disconnected signal
    /// when all required operations are done
    virtual void disconnect(bool terminate) = 0;

    /// Processes terminate request
    virtual void terminate() = 0;

    /// Processes continue request from client
    virtual bool process_continue(size_t thread_id) = 0;

    /// Processes pause request
    virtual void pause(size_t thread_id) = 0;

    /// Processes next request
    virtual void next(size_t thread_id) = 0;

    /// Processes next instruction request
    virtual void next_inst(size_t thread_id) = 0;

    /// Processes step in request
    virtual void step_in(size_t thread_id, size_t target_id) = 0;
    
    /// Processes instruction step in request
    virtual void step_inst_in(size_t thread_id) = 0;

    /// Processes step out request
    virtual void step_out(size_t thread_id) = 0;

    /// Processes threads request
    virtual std::vector<dap::thread> threads() = 0;

    /// Processes stack trace request
    virtual size_t stack_trace(size_t thread_id,
                               size_t first,
                               size_t max_cnt,
                               const dap::stack_frame_format & fmt,
                               std::vector<dap::stack_frame> & res) = 0;

    /// Processes scopes request
    virtual std::vector<dap::scope> scopes(size_t frame_id) = 0;

    /// Processes variables request
    virtual std::vector<dap::variable> variables(size_t var_ref,
                                                 dap::variable_filter filter,
                                                 size_t start,
                                                 size_t count,
                                                 const dap::value_format & fmt) = 0;

    /// Processes evaluate request
    virtual dap::evaluate_response evaluate(const std::wstring & expr,
                                            size_t frame_id,
                                            const std::wstring & context,
                                            const dap::value_format & fmt) = 0;

    /// Processes set breakpoints request
    virtual void set_breakpoints(const path_t & src,
                                 const std::vector<dap::source_breakpoint> & bpts,
                                 const std::vector<size_t> & line_numbers,
                                 bool source_modified) = 0;

    /// Processes set exception breakpoints request
    virtual void set_exception_breakpoints(const std::vector<std::wstring> & filters,
                                           const std::vector<dap::exception_options> & opts) = 0;

    /// Processes set function breakpoints request
    virtual void set_function_breakpoints(const std::vector<dap::function_breakpoint> & bpts) = 0;

    /// Processes set instruction breakpoints request
    virtual void set_instruction_breakpoints(const std::vector<dap::instruction_breakpoint> & bpts) = 0;

    /// Disassembles memory
    virtual std::vector<dap::disassembled_instruction>
    disassemble(uint64_t addr, int64_t inst_offset, size_t inst_count, bool resolve_symbols) = 0;

    /// Reads memory
    virtual dap::read_memory_result read_memory(std::wstring_view mem_ref, int64_t offset, size_t size) = 0;

    /// The signal is emitted when all cleanup tasks are done after disconnect request
    /// and instance is ready to be destroyed
    signal_t<void ()> disconnected;

    /// The signal is emitted when child process exited
    signal_t<void (int)> exited;

    /// The signal is emitted when process is stopped. Arguments passed to signal handler
    /// are same as for send_stopped function in server_impl.
    signal_t<void (const std::string &,
                   const std::string &,
                   size_t,
                   bool,
                   const std::string &,
                   bool,
                   const std::filesystem::path &,
                   size_t line,
                   size_t column)> stopped;

    /// The signal is emitted when stdout output is received from child process
    signal_t<void (const std::string &)> stdout_received;

    /// The signal is emitted when stderr output it received from child process
    signal_t<void (const std::string &)> stderr_received;

    /// The signal is emitted when debugger starts module loading
    signal_t<void (const std::filesystem::path &)> module_load_started;

    /// The signal is emitted when debugger finishes module loading
    signal_t<void (const std::filesystem::path &)> module_load_finished;
};


}
