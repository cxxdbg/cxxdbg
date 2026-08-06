// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file nodebug_child_process.cpp
/// Contains implementation of the nodebug_child_process class.

#include "nodebug_child_process.hpp"


namespace cxxdbg::dbg::dapsrv {


void nodebug_child_process::process_configuration_done(const std::function<void (const std::string &)> & comp_handler) {
    // launching child
    launcher_->launch_async(launch_pars_, [this, comp_handler](auto && child_mon, auto && err) {
        // checking for error
        if (!err.empty() || !child_mon) {
            std::ostringstream msg;
            msg << "error launching process: " << (!err.empty() ? err : std::string{"unknown error"});
            comp_handler(msg.str());
            return;
        }

        // saving child monitor
        child_ = std::move(child_mon);

        // listening for child monitor events
        child_->exited.connect([this](int ecode) {
            // removing child monitor
            child_.reset();

            // notifying client
            exited(ecode);
        });

        // invoking client completion handler
        comp_handler({});
    });
}


void nodebug_child_process::disconnect(bool is_term) {
    if (child_) {
        // if child is running then terminating it first
        exited.connect([this](int ecode) {
            disconnected();
        });

        terminate();
    } else {
        // notifying client that everything is done
        disconnected();
    }
}


void nodebug_child_process::terminate() {
    child_->terminate();
}


bool nodebug_child_process::process_continue(size_t thread_id) {
    throw std::runtime_error{"can't continue nodebug process"};
}


void nodebug_child_process::pause(size_t thread_id) {
    throw std::runtime_error{"can't pause nodebug process"};
}


void nodebug_child_process::next(size_t thread_id) {
    throw std::runtime_error{"can't perform next for nodebug process"};
}


void nodebug_child_process::next_inst(size_t thread_id) {
    throw std::runtime_error{"cant' perform next for nodebug process"};
}


void nodebug_child_process::step_in(size_t thread_id, size_t target_id) {
    throw std::runtime_error{"can't perform step into for nodebug process"};
}


void nodebug_child_process::step_inst_in(size_t thread_id) {
    throw std::runtime_error{"can't perform step into for nodebug process"};
}


void nodebug_child_process::step_out(size_t thread_id) {
    throw std::runtime_error{"can't perform step out nodebug process"};
}


std::vector<dap::thread> nodebug_child_process::threads() {
    // clients try get threads even for nodebug process. We should return empty vector here
    return {};
}


size_t nodebug_child_process::stack_trace(size_t thread_id,
                                          size_t first,
                                          size_t max_cnt,
                                          const dap::stack_frame_format & fmt,
                                          std::vector<dap::stack_frame> & res) {
    throw std::runtime_error{"can't get stack trace for nodebug process"};
}


std::vector<dap::scope> nodebug_child_process::scopes(size_t frame_id) {
    throw std::runtime_error{"can't get scopes for nodebug process"};
}


std::vector<dap::variable> nodebug_child_process::variables(size_t var_ref,
                                                            dap::variable_filter filter,
                                                            size_t start,
                                                            size_t count,
                                                            const dap::value_format & fmt) {
    throw std::runtime_error{"can't get variables for nodebug process"};
}


dap::evaluate_response nodebug_child_process::evaluate(const std::wstring & expr,
                                                       size_t frame_id,
                                                       const std::wstring & context,
                                                       const dap::value_format & fmt) {
    throw std::runtime_error{"can't evaluate expression for nodebug process"};
}


void nodebug_child_process::set_breakpoints(const path_t & src,
                                            const std::vector<dap::source_breakpoint> & bpts,
                                            const std::vector<size_t> & line_numbers,
                                            bool source_modified) {
    // clients try set breakpoints even for nodebug. Doing nothing here
}


void nodebug_child_process::set_exception_breakpoints(const std::vector<std::wstring> & filters,
                                                      const std::vector<dap::exception_options> & opts) {
    // clients try set breakpoints even for nodebug. Doing nothing here
}


void nodebug_child_process::set_function_breakpoints(const std::vector<dap::function_breakpoint> & bpts) {
    // clients try set breakpoints even for nodebug. Doing nothing here
}


void nodebug_child_process::set_instruction_breakpoints(const std::vector<dap::instruction_breakpoint> & bpts) {
    // clients try set breakpoints even for nodebug. Doing nothing here
}


std::vector<dap::disassembled_instruction>
nodebug_child_process::disassemble(uint64_t addr, int64_t inst_offset, size_t inst_count, bool resolve_symbols) {
    throw std::runtime_error{"can't disassemblefor nodebug process"};
}


dap::read_memory_result nodebug_child_process::read_memory(std::wstring_view mem_ref, int64_t offset, size_t size) {
    throw std::runtime_error{"can't read memory from process launched without debugging"};
}


}
