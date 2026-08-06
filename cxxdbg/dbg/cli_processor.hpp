// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file cli_processor.hpp
/// Contains definition of the cli_processor class.

#pragma once

#include "breakpoint.hpp"
#include "cxxdbg/cli/app_processor.hpp"


namespace cxxdbg::dbg {


class debugger;


/// Implementation of command processor for command line interpreter
/// which passes commands to debugger instance. Implement all methods
/// except target management (load/unload/launch/terminate)
class cli_processor: public cli::app_processor {
public:
    /// Constructor, makes processor with specified reference to debugger instance
    cli_processor(debugger & dbg);

    /// Deletes breakpoint with specified index
    void delete_breakpoint(const breakpoint_id & id, const async::result_handler<> & h) override;

    /// Enabled breakpoint with specified index
    void enable_breakpoint(const breakpoint_id & id, const async::result_handler<> & h) override;

    /// Disables breakpoint with specified index
    void disable_breakpoint(const breakpoint_id & id, const async::result_handler<> & h) override;

    /// Adds breakpoint at function with specified name
    void add_function_breakpoint(const std::string & name,
                                 const cli::break_properties & props,
                                 const code_break_handler & handl) override;

    /// Adds breakpoint when exception thrown
    void add_exception_thrown_breakpoint(const cli::break_properties & props,
                                         const code_break_handler & handl) override;

    /// Adds breakpoint when exception caught
    void add_exception_caught_breakpoint(const cli::break_properties & props,
                                         const code_break_handler & handl) override;

    /// Adds breakpoint at specified source position
    void add_srcpos_breakpoint(const std::filesystem::path & file,
                               unsigned int line,
                               const cli::break_properties & props,
                               const code_break_handler & handl) override;

    /// Adds breakpoint at specified address
    void add_address_breakpoint(unsigned long addr,
                                const cli::break_properties & props,
                                const code_break_handler & handl) override;

    /// Sets breakpoint properties
    void set_breakpoint_props(const breakpoint_id & id,
                              const cli::break_properties & props,
                              const async::result_handler<> & h) override;

    /// Prints list of breakpoints of specified type and returns it as string
    std::string list_breakpoints(type_t type) override;

    /// Adds watchpoint for variable with specified name
    void add_var_watchpoint(const std::string & name,
                            const cli::watch_create_properties & props,
                            const watch_handler & handl) override;

    /// Adds watchpoint for value pointed by address specified by expression
    void add_expr_watchpoint(const std::string & expr,
                             const cli::watch_create_properties & props,
                             const watch_handler & handl) override;

    /// Executes command with specified command string and handler
    void exec_cmd(const std::string & cmd, const result_handler & handl) override;

    /// Selects frame with specified index
    void select_frame(std::size_t index, const async::result_handler<> &) override;

    /// Continues execution of current process
    void resume(const process_handler & handler) override;

    /// Interrupts current process
    void interrupt(const process_handler & handler) override;

    /// Selects active thread
    void select_thread(std::size_t index, const async::result_handler<> & h) override;

    /// Performs step into action
    void step_into(tribool avoid_nodebug,
                   const std::string & target_func,
                   const std::string & avoid_regex,
                   const std::string & step_through_regex,
                   const async::result_handler<> & h) override;

    /// Performs step over action
    void step_over(const std::string & step_through_regex,
                   const async::result_handler<> & h) override;

    /// Performs step out action
    void step_out(tribool avoid_nodebug,
                  const std::string & step_through_regex,
                  const async::result_handler<> & h) override;

    /// Performs instruction step into action
    void inst_step_into(const async::result_handler<> & h) override;

    /// Performs instruction step over action
    void inst_step_over(const async::result_handler<> & h) override;

    /// Performs step until action
    void step_until(unsigned int line, const async::result_handler<> & h) override;

private:
    debugger & dbg_;        ///< Reference to debugger object
};


}


