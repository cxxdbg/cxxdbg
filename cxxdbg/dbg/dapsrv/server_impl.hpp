// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file server_impl.hpp
/// Contains definition of the server_impl class.

#pragma once

#include "asio_event_queue.hpp"
#include "child_process.hpp"
#include "stack_frame_compare.hpp"
#include "variable_reference.hpp"
#include "cxxdbg/app/application_log.hpp"
#include "cxxdbg/app/signals.hpp"
#include "cxxdbg/dbg/core/debugger.hpp"
#include "cxxdbg/dbg/core/target.hpp"
#include <cxxdap/server_impl.hpp>
#include <boost/asio/executor.hpp>


namespace cxxdbg::dbg::dapsrv {


class client_term_process_launcher;


/// DAP server implementation for cxxdbg
class server_impl: virtual public dap::server_impl {
    friend class client_term_process_launcher;
    friend class client_term_exec_stub_launcher;

public:
    /// Constructs server implementation with specified reference to ASIO executor for
    /// executing code in work thread
    server_impl(const boost::asio::executor & exec): exec_{exec} {}

    ~server_impl() override;

    /// Handles closing of input stream. Starts debugger termination and invokes
    /// notify_exited when all cleanup is done
    void handle_input_closed() override;

    /// Initializes server implementation. Returns initialize response
    dap::initialize_response initialize(const dap::initialize_request & req) override;

    /// Terminates or disconnects from debuggee
    void disconnect(bool restart, bool terminate) override;

    /// Processes launch request from client
    void launch(bool no_debug,
                const dap::json_t & parameters,
                const std::function<void (const std::string &)> & comp_handler) override;

    /// Processes attach request from client
    void attach(const dap::json_t & parameters) override;

    /// Processes continue request from client
    bool process_continue(size_t thread_id) override;

    /// Processes configuration done request from client
    void configuration_done(const std::function<void (const std::string &)> & comp_handler) override;

    /// Processes completion request. Returns vector of completion items
    std::vector<dap::completion_item>
    completions(size_t frame_id, const std::wstring & text, size_t column, size_t line) override;

    /// Processes data breakpoint info request
    dap::data_breakpoint_info get_data_breakpoint_info(size_t var, const std::wstring & name) override;

    /// Processes evaluate request
    dap::evaluate_response evaluate(const std::wstring & expr,
                                    size_t frame_id,
                                    const std::wstring & context,
                                    const dap::value_format & fmt) override;

    /// Processes goto request
    void process_goto(size_t thread_id, size_t target_id) override;

    /// Processes goto targets request
    std::vector<dap::goto_target> goto_targets(const path_t & src, size_t line, size_t column) override;

    /// Processes loaded sources request
    std::vector<path_t> loaded_sources() override;

    /// Porcesses modules request. Returns total number of modules available
    size_t modules(size_t start, size_t cnt, std::vector<dap::module> & res) override;

    /// Processes next request
    void next(size_t thread_id, dap::stepping_granularity granularity) override;

    /// Processes pause request
    void pause(size_t thread_id) override;

    /// Processes read memory request
    dap::read_memory_result read_memory(std::wstring_view mem_ref, int64_t offset, uint64_t cnt) override;

    /// Processes scopes request
    std::vector<dap::scope> scopes(size_t frame_id) override;

    // /// Processes source request
    // std::wstring source(size_t src_id, std::wstring & mime_type) override;

    /// Sets breakpoints in source. Returns vector of breakpoints set.
    std::vector<dap::breakpoint>
    set_breakpoints(const path_t & src,
                    const std::vector<dap::source_breakpoint> & bpts,
                    const std::vector<size_t> & line_numbers,
                    bool source_modified) override;

    /// Processes set data breakpoints request
    std::vector<dap::breakpoint> set_data_breakpoints(const std::vector<dap::data_breakpoint> & bpts) override;

    /// Processes set exception breakpoints request
    void set_exception_breakpoints(const std::vector<std::wstring> & filters,
                                   const std::vector<dap::exception_options> & opts) override;

    /// Processes set function breakpoints request
    std::vector<dap::breakpoint> set_function_breakpoints(const std::vector<dap::function_breakpoint> & bpts) override;

    /// Processes set instruction breakpoints request
    std::vector<dap::breakpoint> set_instruction_breakpoints(const std::vector<dap::instruction_breakpoint> & bpts) override;

    /// Processes stack trace request
    size_t stack_trace(size_t thread_id,
                       size_t first,
                       size_t max_cnt,
                       const dap::stack_frame_format & fmt,
                       std::vector<dap::stack_frame> & res) override;

    /// Processes step in request
    void step_in(size_t thread_id, size_t target_id, dap::stepping_granularity granularity) override;

    /// Processes step in targets request
    std::vector<dap::step_in_target> step_in_targets(size_t frame_id) override;

    /// Processes step out request
    void step_out(size_t thread_id, dap::stepping_granularity granularity) override;

    /// Processes restart request
    void restart() override;

    /// Processes terminate request
    void terminate(bool restart) override;

    /// Processes threads request
    std::vector<dap::thread> threads() override;

    /// Processes variables request
    std::vector<dap::variable> variables(size_t var_ref,
                                         dap::variable_filter filter,
                                         size_t start,
                                         size_t count,
                                         const dap::value_format & fmt) override;

    /// Processes disassemble request
    std::vector<dap::disassembled_instruction> disassemble(std::wstring_view mem_ref,
                                                           int64_t offset,
                                                           int64_t inst_offset,
                                                           uint64_t inst_count,
                                                           bool resolve_symbols) override;

private:
    /// Returns const reference to executor for work thread
    const auto & get_executor() { return exec_; }

    /// Connects to all child process signals
    void prepare_child();

    /// Returns reference to child process. Throws exception
    /// if child does not exist
    child_process & child();

    boost::asio::executor exec_;            ///< ASIO executor for executing code in work thread
    application_log log_{"dap"};            ///< Application log (with ignored "dap" category)
    scoped_signal_connection log_con_;      ///< Connection to log events
    bool disconnect_requested_ = false;     ///< Was disconnect requested?

    /// Map of source breakpoints to be added after loading target
    std::map<std::filesystem::path, std::vector<std::tuple<size_t, dap::source_breakpoint>>> source_breakpoints_;

    /// Vector of function breakpoints to be added after loading target
    std::vector<std::tuple<size_t, dap::function_breakpoint>> function_breakpoints_;

    /// Vector of instruction breakpoints to be added after loading target
    std::vector<std::tuple<size_t, dap::instruction_breakpoint>> instruction_breakpoints_;

    /// Vector of exception filters to be added after loading target
    std::vector<std::wstring> exception_breakpoint_filters_;

    /// Next DAP breakpoint ID
    size_t next_dap_bp_id_ = 1;

    /// Pointer to child launched by DAP server
    std::unique_ptr<child_process> child_;
};


}
