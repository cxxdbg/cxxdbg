// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file nodebug_child_process.hpp
/// Contains definition of the nodebug_child_process class.

#pragma once

#include "child_process.hpp"
#include "cxxdbg/proc/async_launcher.hpp"


namespace cxxdbg::dbg::dapsrv {


/// Base class for all child processes launched with nodebug
class nodebug_child_process: virtual public child_process {
public:
    /// Constructs process with specified process launcher and launch parameters
    explicit nodebug_child_process(std::unique_ptr<proc::async_launcher> && launcher,
                                   proc::launch_parameters pars):
        launcher_{std::move(launcher)}, launch_pars_{std::move(pars)} {}

    /// Processes configuration done request. Launches process for debugging or resumes attached process
    void process_configuration_done(const std::function<void (const std::string &)> & comp_handler) override;

    /// Processes disconnect request. Terminates child if needed and emits disconnected signal
    /// when all required operations are done
    void disconnect(bool terminate) override;

    /// Processes terminate request
    void terminate() override;

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

    /// Disassembles memory
    std::vector<dap::disassembled_instruction>
    disassemble(uint64_t addr, int64_t inst_offset, size_t inst_count, bool resolve_symbols) override;

    /// Reads memory
    dap::read_memory_result read_memory(std::wstring_view mem_ref, int64_t offset, size_t size) override;

private:
    /// Process launcher
    std::unique_ptr<proc::async_launcher> launcher_;

    /// Launch parameters saved in prepare launch
    proc::launch_parameters launch_pars_;

    /// Process monitor
    std::unique_ptr<proc::monitor> child_;
};


}
