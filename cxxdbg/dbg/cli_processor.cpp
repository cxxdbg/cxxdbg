// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file cli_processor.cpp
/// Contains implementation of the cli_processor class.

#include "cli_processor.hpp"
#include "application.hpp"
#include "breakpoint_list.hpp"
#include "code_breakpoint.hpp"
#include "debugger.hpp"
#include "target.hpp"
#include "thread.hpp"
#include "thread_list.hpp"
#include "watchpoint.hpp"
#include <cassert>


namespace cxxdbg::dbg {


cli_processor::cli_processor(debugger & dbg):
dbg_{dbg} {
}


/// Checks application state for breakpoint command, returns error message
static std::string check_dbg_bp_state(debugger & dbg) {
    if (dbg.state() == debugger::state_t::initial) {
        return "target not loaded";
    }

    if (dbg.state() != debugger::state_t::stopped &&
        dbg.state() != debugger::state_t::loaded) {
        return "process is not stopped";
    }

    return std::string{};
}


/// Checks application state for watchpoint adding commands, returns error message
static std::string check_dbg_wp_state(debugger & dbg) {
    if (dbg.state() != debugger::state_t::stopped) {
        return "target is not running";
    }

    return {};
}


void cli_processor::delete_breakpoint(const breakpoint_id & id, const async::result_handler<> & h) {
    std::string err = check_dbg_bp_state(dbg_);
    if (!err.empty()) {
        h(async::result<>{err});
    }

    const breakpoint * bp = dbg_.breakpoints().find_breakpoint(id);
    if (bp == nullptr) {
        h(async::result<>{"breakpoint not found"});
        return;
    }

    dbg_.breakpoints().remove_breakpoint(bp);
    h(async::result<>{});
}


void cli_processor::enable_breakpoint(const breakpoint_id & id, const async::result_handler<> & h) {
    std::string err = check_dbg_bp_state(dbg_);
    if (!err.empty()) {
        h(async::result<>{err});
    }

    const breakpoint * bp = dbg_.breakpoints().find_breakpoint(id);
    if (bp == nullptr) {
        h(async::result<>{"breakpoint not found"});
        return;
    }

    dbg_.breakpoints().enable_breakpoint(bp);
    h(async::result<>{});
}


void cli_processor::disable_breakpoint(const breakpoint_id & id, const async::result_handler<> & h) {
    std::string err = check_dbg_bp_state(dbg_);
    if (!err.empty()) {
        h(async::result<>{err});
    }

    const breakpoint * bp = dbg_.breakpoints().find_breakpoint(id);
    if (bp == nullptr) {
        h(async::result<>{"breakpoint not found"});
        return;
    }

    dbg_.breakpoints().disable_breakpoint(bp);
    h(async::result<>{});
}


/// Updates breakpoint properties
static void update_bp_props(debugger & dbg,
                            const breakpoint * bp,
                            const cli::break_properties & props) {
    if (!props.cond.empty()) {
        dbg.breakpoints().set_breakpoint_condition(bp, props.cond);
    }

    if (props.enable_hit_count) {
        dbg.breakpoints().set_breakpoint_hit_count(bp, props.enable_hit_count, props.hit_count, false);
    }
}


void cli_processor::add_function_breakpoint(const std::string & name,
                                            const cli::break_properties & props,
                                            const code_break_handler & h) {
    std::string err = check_dbg_bp_state(dbg_);
    if (!err.empty()) {
        h(async::result<unsigned long>{err});
    }

    dbg_.breakpoints().add_breakpoint(name, [this, h, props](auto && bp) {
        update_bp_props(dbg_, bp, props);
        h(async::ok_result(bp->num()));
    });
}

void cli_processor::add_exception_thrown_breakpoint(const cli::break_properties & props,
                                                    const code_break_handler & handl) {
    std::string err = check_dbg_bp_state(dbg_);
    if (!err.empty()) {
        handl(async::result<unsigned long>{err});
    }

    dbg_.breakpoints().add_breakpoint_exception_thrown([this, handl, props](auto && bp) {
        update_bp_props(dbg_, bp, props);
        handl(async::ok_result(bp->num()));
    });
}

void cli_processor::add_exception_caught_breakpoint(const cli::break_properties & props,
                                                    const code_break_handler & handl) {
    std::string err = check_dbg_bp_state(dbg_);
    if (!err.empty()) {
        handl(async::result<unsigned long>{err});
    }

    dbg_.breakpoints().add_breakpoint_exception_caught([this, handl, props](auto && bp) {
        update_bp_props(dbg_, bp, props);
        handl(async::ok_result(bp->num()));
    });
}


void cli_processor::add_srcpos_breakpoint(const std::filesystem::path & file,
                                          unsigned int line,
                                          const cli::break_properties & props,
                                          const code_break_handler & h) {
    std::string err = check_dbg_bp_state(dbg_);
    if (!err.empty()) {
        h(async::result<unsigned long>{err});
    }

    dbg_.breakpoints().add_breakpoint(source_position_info{file, line}, [this, h, props](auto && bp) {
        update_bp_props(dbg_, bp, props);
        h(async::ok_result(bp->num()));
    });
}


void cli_processor::add_address_breakpoint(unsigned long addr,
                                           const cli::break_properties & props,
                                           const code_break_handler & h) {
    std::string err = check_dbg_bp_state(dbg_);
    if (!err.empty()) {
        h(async::result<unsigned long>{err});
    }

    dbg_.breakpoints().add_breakpoint(addr, [this, h, props](auto && bp) {
        update_bp_props(dbg_, bp, props);
        h(async::ok_result(bp->num()));
    });
}


void cli_processor::set_breakpoint_props(const breakpoint_id & id,
                                         const cli::break_properties & props,
                                         const async::result_handler<> & h) {
    std::string err = check_dbg_bp_state(dbg_);
    if (!err.empty()) {
        h(async::result<>{err});
    }

    auto bp = dbg_.breakpoints().find_breakpoint(id);
    if (bp == nullptr) {
        h(async::result<>{"breakpoint not found"});
        return;
    }

    update_bp_props(dbg_, bp, props);

    h(async::result<>{});
}


std::string cli_processor::list_breakpoints(cli::break_processor::type_t type) {
    if (!dbg_.has_target()) {
        return "No target loaded";
    }

    std::ostringstream str;
    auto bps = dbg_.breakpoints().all_of_type(type);
    for (auto && bp : bps) {
        str << bp->num()
            << " name = '" << bp->name() << "'"
            << " hit count = " << bp->curr_hit_count() << "\n";
    }

    return str.str();
}


void cli_processor::add_var_watchpoint(const std::string & name,
                                       const cli::watch_create_properties & props,
                                       const watch_handler & handl) {
    auto err = check_dbg_wp_state(dbg_);
    if (!err.empty()) {
        handl({err});
        return;
    }

    dbg_.breakpoints().add_var_watchpoint(name, props.is_read, props.is_write, props.size,
    [handl](auto && res) {
        if (res.is_ok()) {
            handl({res.value()->num()});
        } else {
            handl({res.error()});
        }
    });
}


void cli_processor::add_expr_watchpoint(const std::string & expr,
                                        const cli::watch_create_properties & props,
                                        const watch_handler & handl) {
    auto err = check_dbg_wp_state(dbg_);
    if (!err.empty()) {
        handl({err});
        return;
    }

    dbg_.breakpoints().add_expr_watchpoint(expr, props.is_read, props.is_write, props.size,
    [handl](auto && res) {
        if (res.is_ok()) {
            handl({res.value()->num()});
        } else {
            handl({res.error()});
        }
    });
}


void cli_processor::exec_cmd(const std::string & cmd, const result_handler & handl) {
    dbg_.execute(cmd, handl);
}


void cli_processor::select_frame(std::size_t index, const async::result_handler<> & handler) {
    // checking that process is stopped
    if (dbg_.state() != application::state_t::stopped) {
        handler(async::result<>{"process is not in stopped state"});
        return;
    }

    if (index >= dbg_.current_thread()->call_stack_size()) {
        handler(async::result<>{"frame with specified index does not exist"});
        return;
    }

    dbg_.set_current_frame(dbg_.current_thread()->frame_at(index));

    std::ostringstream res;
    res << "selected frame " << index;
    handler(async::result<>{res.str()});
}


void cli_processor::resume(const process_handler & handler) {
    if (dbg_.state() != debugger::state_t::stopped) {
        handler(async::result<unsigned long>("process is not in stopped state"));
        return;
    }

    dbg_.run();
    handler(async::ok_result(dbg_.current_target().pid()));
}


void cli_processor::interrupt(const process_handler & handler) {
    if (dbg_.state() != debugger::state_t::running) {
        handler(async::result<unsigned long>("process is not running"));
        return;
    }

    dbg_.stop();
    handler(async::ok_result(dbg_.current_target().pid()));
}


void cli_processor::select_thread(std::size_t index, const async::result_handler<> & h) {
    if (dbg_.state() != debugger::state_t::stopped) {
        h(async::result<>{"process is not in stopped state"});
        return;
    }

    if (index >= dbg_.current_target().threads().size()) {
        h(async::result<>{"thread with specified index does not exist"});
        return;
    }

    const thread * thrd = dbg_.current_target().threads().at(index);
    dbg_.set_current_thread(thrd);

    std::ostringstream msg;
    msg << "selected thread " << index;
    h(msg.str());
}


void cli_processor::step_into(tribool avoid_nodebug,
                              const std::string & target_func,
                              const std::string & avoid_regex,
                              const std::string & step_through_regex,
                              const async::result_handler<> & h) {
    if (dbg_.state() != debugger::state_t::stopped) {
        h(async::result<>{"process is not in stopped state"});
        return;
    }

    dbg_.step_into(avoid_nodebug, target_func, avoid_regex, step_through_regex);
    h(async::result<>{});
}


void cli_processor::step_over(const std::string & step_through_regex,
                              const async::result_handler<> & h) {
    if (dbg_.state() != debugger::state_t::stopped) {
        h(async::result<>{"process is not in stopped state"});
        return;
    }

    dbg_.step_over(step_through_regex);
    h(async::result<>{});
}


void cli_processor::step_out(tribool avoid_nodebug,
                             const std::string & step_through_regex,
                             const async::result_handler<> & h) {
    if (dbg_.state() != debugger::state_t::stopped) {
        h(async::result<>{"process is not in stopped state"});
        return;
    }

    dbg_.step_out(avoid_nodebug, step_through_regex);
    h(async::result<>{});
}


void cli_processor::inst_step_into(const async::result_handler<> & h) {
    if (dbg_.state() != debugger::state_t::stopped) {
        h(async::result<>{"process is not in stopped state"});
        return;
    }

    dbg_.step_inst_into();
    h(async::result<>{});
}


void cli_processor::inst_step_over(const async::result_handler<> & h) {
    if (dbg_.state() != debugger::state_t::stopped) {
        h(async::result<>{"process is not in stopped state"});
        return;
    }

    dbg_.step_inst_over();
    h(async::result<>{});
}


void cli_processor::step_until(unsigned int line, const async::result_handler<> & h) {
    if (dbg_.state() != debugger::state_t::stopped) {
        h(async::result<>{"process is not in stopped state"});
        return;
    }

    dbg_.step_until(line);
    h(async::result<>{});
}


}
