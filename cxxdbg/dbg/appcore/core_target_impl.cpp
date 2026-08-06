// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file core_target_impl.cpp
/// Contains implementation of core_target_impl class.

#include "core_target_impl.hpp"
#include "core_locals_list_impl.hpp"
#include "core_module_impl.hpp"
#include "log.hpp"
#include "utils.hpp"
#include "cxxdbg/async/async_execution_queue.hpp"
#include "cxxdbg/dbg/breakpoint_info.hpp"
#include "cxxdbg/dbg/breakpoint_location_info.hpp"
#include "cxxdbg/dbg/code_position_info.hpp"
#include "cxxdbg/dbg/launch_options.hpp"
#include "cxxdbg/dbg/module_info.hpp"
#include "cxxdbg/dbg/process_context.hpp"
#include "cxxdbg/dbg/stack_frame_info.hpp"
#include "cxxdbg/dbg/thread_info.hpp"
#include "cxxdbg/dbg/watch_list_impl.hpp"
#include "cxxdbg/dbg/core/format.hpp"
#include "cxxdbg/dbg/core/variable.hpp"
#include "cxxdbg/dbg/core/debugger.hpp"
#include "dbgfmt/context.hpp"
#include "dbgfmt/fmt_result.hpp"
#include "cxxdbg/util/convert.hpp"

#include <ranges.hpp>
#include <utility>
#include <iostream>


namespace fs = std::filesystem;


namespace cxxdbg::dbg::appcore {


static constexpr size_t max_frames = 50;


/// Converts core breakpoint location to breakpoint_location_info object
static dbg::breakpoint_location_info
conv_bp_loc(const core::breakpoint_location & loc, const core::target & targ) {
    return dbg::breakpoint_location_info(loc.id(), extract_code_position_info(targ, loc.pos(), loc.pos()));
}


/// Extracts common code breakpoint and watchpoint properties and stores them
/// to breakpoint_info
static void convert_breakpoint_properties(const core::breakpoint * bp,
                                          dbg::breakpoint_info & info) {
    // setting current hit count
    info.set_curr_hit_count(bp->curr_hit_count());
}


/// Converts code breakpoint to code_breakpoint_info object
static std::shared_ptr<dbg::code_breakpoint_info>
convert_code_breakpoint(const core::code_breakpoint * bp, const core::target & targ) {
    auto bp_info = std::make_shared<dbg::code_breakpoint_info>(bp->id().num());

    // adding breakpoint locations
    for (auto it = bp->locations_begin(), end = bp->locations_end();
         it != end; ++it) {
        bp_info->add_location(conv_bp_loc(*it, targ));
    }

    // converting common poperties
    convert_breakpoint_properties(bp, *bp_info);

    return bp_info;
}


/// Converts watchpoint to watchpoint_info object
static std::shared_ptr<dbg::watchpoint_info>
convert_watchpoint(const core::watchpoint * wp) {
    auto info = std::make_shared<dbg::watchpoint_info>(wp->id().num());

    convert_breakpoint_properties(wp, *info);

    auto ov = wp->old_value();
    auto nv = wp->new_value();

    info->set_old_val(std::wstring(ov.begin(), ov.end()));
    info->set_new_val(std::wstring(nv.begin(), nv.end()));
    info->set_watch_addr(wp->watch_addr());

    return info;
}


/// Converts core breakpoint to breakpoint_info object
static std::shared_ptr<dbg::breakpoint_info>
convert_breakpoint(const core::breakpoint * bp, const core::target & targ) {
    if (auto code_bp = dynamic_cast<const core::code_breakpoint*>(bp)) {
        return convert_code_breakpoint(code_bp, targ);
    } else if (auto wp = dynamic_cast<const core::watchpoint*>(bp)) {
        return convert_watchpoint(wp);
    } else {
        assert(false && "Unknown breakpoint type");
        return {};
    }
}


/// Converts core call parameter to call_parameter_info object
static dbg::call_parameter_info convert_call_parameter(const dbgfmt::named_fmt_result & par) {
    return dbg::call_parameter_info(par.name(), par.type(), par.val());
}


/// Converts core stack frame to stack_frame_info object
static dbg::stack_frame_info convert_core_stack_frame(bool zero,
                                                      core::stack_frame & frame,
                                                      core::target & targ) {
    // for non-zero frames offset position by -1
    core::code_position pos = frame.read_pos();
    core::code_position call_pos = pos;
    if (!zero) {
        call_pos = pos.offset(-1);
    }

    dbg::stack_frame_info frame_info{extract_code_position_info(targ, pos, call_pos),
                                     frame.get_cfa(),
                                     frame.get_sc_id()};

    auto params = frame.read_params_vars();
    for (auto par : params) {
        auto par_val = format_variable(targ, frame, par);
        frame_info.add_param(convert_call_parameter(par_val));
    }

    return frame_info;
}


/// Converts core thread to thread_info object
static dbg::thread_info convert_core_thread(core::thread & thrd,
                                            core::target & targ,
                                            bool fetch_all) {
    dbg::thread_info tinfo(thrd.id());

    std::size_t nframes = 0;
    tinfo.set_has_complete_call_stack(true);
    tinfo.set_curr_frame_index(thrd.selected_frame_index());

    bool is_first = true;

    for (auto && frm: thrd.frames() | std::ranges::views::common) {
        if (!fetch_all && nframes >= max_frames) {
            tinfo.set_has_complete_call_stack(false);
            break;
        }

        // *it may return invalid frame. TODO: refactor
        if (!frm.is_valid())
            break;

        tinfo.add_stack_frame(convert_core_stack_frame(is_first, frm, targ));
        ++nframes;
        is_first = false;
    }

    // trying extract vector of call targets in thread position
    if (nframes > 0) {
        auto first_frame = thrd.frame(0);
        if (first_frame.is_valid()) {
            auto code_pos = first_frame.read_pos();
            if (code_pos) {
                tinfo.set_call_targets(targ.call_targets(code_pos));
            }
        }
    }

    return tinfo;
}


core_target_impl::core_target_impl(core::debugger & dbg,
                                   async::event_queue & m_q,
                                   async::execution_queue & c_a_q,
                                   const std::shared_ptr<core::target> & tgt,
                                   const std::vector<dbg::module_info> & mods):
dbg_{dbg},
main_thread_queue_(m_q),
core_async_queue_(c_a_q),
targ_(tgt),
state_(state_t::loaded),
locals_(core_async_queue_, *targ_),
watch_{core_async_queue_, *targ_},
registers_{core_async_queue_, *targ_} {
    // adding all modules which were loaded after loading target
    for (auto m : mods) {
        mod_list_.insert(m.mod_path());
    }

    // changing state to lunching if core target state is launching
    // or stopped (for attaching to process)
    switch (targ_->state()){
    case core::target::state_t::loaded:
        // doing nothing
        break;
    case core::target::state_t::launching:
        state_ = state_t::launching;
        break;
    case core::target::state_t::stopped:
        state_ = state_t::stopped;
        break;
    default:
        assert(false && "Unknown target state when constructing app target impl");
        break;
    }

    // connecting to core state changed signal
    core_state_changed_con_ = targ_->connect_state_changed([this](core::target::state_t st) {
        on_core_state_changed(st);
    });


    // listening modules signals

    core_modules_loaded_con_ = targ_->modules_loaded.connect([this](auto && mods) {
        on_core_modules_loaded(mods);
    });

    core_modules_unloaded_con_ = targ_->modules_unloaded.connect([this](auto && mods) {
        on_core_modules_unloaded(mods);
    });


    // listening for stdout signal
    core_stdout_con_ = targ_->stdout_received().connect([this](const std::string & data) {
        // core thread
        main_thread_queue_.post([this, data] {
            // main thread
            stdout_received()(data);
        });
    });


    // listening for stderr signal
    core_stderr_con_ = targ_->stderr_received().connect([this](const std::string & data) {
        // core thread
        main_thread_queue_.post([this, data] {
            // main thread
            stderr_received()(data);
        });
    });
}


core_target_impl::~core_target_impl() {
}


signal_connection core_target_impl::connect_state_changed(const state_changed_handler & handl) {
    return state_changed_signal_.connect(handl);
}


core_target_impl::state_t core_target_impl::state() const {
    return state_;
}


void core_target_impl::launch(const dbg::launch_options & lopts,
                              const async::result_handler<unsigned long> & handler) {
    assert(state() == state_t::loaded && "Target should be in loaded state");

    // set state to launching
    set_state(state_t::launching);

    // launch lldb target asynchronously
    core_async_queue_.add_command(
    [this, handler](const auto & res) {
        // we should clear is_launching_ flag, change target state and call result handler
        // after all events posted by launch process were processed and ignored, so we do that in
        // action posted via the post_lldb_event function of the debugger
        CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "launch complete, posting launch completion handler via lldb event";
        dbg_.post_lldb_event([this, handler, res] {
            // executed in core thread

            // clearing launching flag
            is_launching_ = false;

            // clearing is_stopped_after_launching flag
            auto is_stopped = is_stopped_after_launching_;
            is_stopped_after_launching_ = false;

            CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "posting launch completion handler to main thread";

            // chaning state and executing handler in main thread
            main_thread_queue_.post([this, res, is_stopped, handler] {
                // checking EH error
                if(!res.is_ok()) {
                    CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "launch error, setting state to loaded";
                    this->set_state(state_t::loaded);

                    CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "executing launch handler with error";
                    handler(res);
                    return;
                }

                // the state after launching is always running when executing completion handler
                // even if target was stopped directly after launching
                this->set_state(state_t::running);

                // executing launch completion hanler
                CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "executing launch handler";
                handler(res);

                // setting target state to stopped if it was stopped during launch process
                if (is_stopped) {
                    CXXDBG_LOG_SCAT_DEBUG(appcore, target) <<
                        "is_stopped_after_launching was true, setting state to stopped";

                    // we have to call process_stopped method to collect process context in
                    // the core thread, so post action to the core queue
                    core_async_queue_.add_command_eh(
                    [this] {},
                    [this] {
                        process_stopped();
                    });
                } else {
                    CXXDBG_LOG_SCAT_DEBUG(appcore, target) <<
                        "is_stopped_after_launching was false, continue with running state";
                }
            });
        });
    },
    [this, lopts]() {
        // Setting launching flag. The flag is used in state changed signal processing to
        // ignore state changed events while process is launching.
        is_launching_ = true;

        // launching target
        auto res = targ_->launch(lopts.work_dir(), lopts.launch_args());

        return res;
    });
}


void core_target_impl::attach(const std::variant<unsigned long, std::string> & t,
                              const async::result_handler<> & h) {

    assert(state() == state_t::loaded && "Target should be in loaded state");

    // set state to launching
    set_state(state_t::launching);

    // attach core target asynchronously
    core_async_queue_.add_command(
    [this, h](const async::result<> & res) {
        // setting state to loaded if attach failed
        if (!res.is_ok()) {
            set_state(state_t::loaded);
        }

        // executing completion handler
        h(res);
    },
    [this, t]() {
        targ_->attach(t);
    });
}


void core_target_impl::run() {
    // can be called only in stopped or running state
    assert((state() == state_t::stopped || state() == state_t::running) &&
           "target::run can be called only in stopped state");

    // changing state to running
    set_state(state_t::running);

    // starting lldb process asynchronously
    core_async_queue_.add_command(
    [this](const async::result<void> & res) {
        // check EH error
        if(!res.is_ok()) {
            set_state(state_t::stopped);
            throw std::runtime_error(res.error());
        }
    },
    [this]() {
        targ_->run();
    });
}


void core_target_impl::stop() {
    // can be called only in stopped or running state
    assert((state() == state_t::running || state() == state_t::stopped)
           && "Invalid target state");

    // stopping lldb process asynchronously
    core_async_queue_.add_command_eh([] {}, [this] { targ_->stop(); });
}


void core_target_impl::terminate() {
    // can be called only in stopped state
    assert(state() == state_t::stopped &&
           "target::terminate can be called only in stopped state");

    // changing state to terminating
    set_state(state_t::terminating);

    // killing lldb process asynchronously
    core_async_queue_.add_command_eh([] {}, [this]() { targ_->terminate(); });
}


void core_target_impl::detach() {
    // can be called only in stopped state
    assert(state() == state_t::stopped &&
           "target::detach can be called only in stopped state");

    // changing state to detaching
    set_state(state_t::detaching);

    // detaching from process asynchronously
    core_async_queue_.add_command_eh([] {}, [this]() { targ_->detach(); });
}


void core_target_impl::close(const std::function<void()> & handl) {
    assert(state() == state_t::loaded &&
           "target::close can be called only in loaded state");

    // change target state to unloading
    set_state(state_t::unloading);

    // unload lldb target asynchronously
    core_async_queue_.add_command_eh(
    [this, handl]() {
        // change target state to invalid
        assert(state() == state_t::unloading && "Invalid target state");
        set_state(state_t::invalid);

        // calling completion handler
        handl();
    },
    [this]() {
        targ_.reset();
    });
}


void core_target_impl::step_into(unsigned long thread_id,
                                 bool avoid_nodebug,
                                 const std::string & target_func,
                                 const std::string & avoid_regex,
                                 const std::string & step_through_regex) {
    auto action = [avoid_nodebug, target_func,
                   avoid_regex, step_through_regex](auto & target, auto thread_id) {
        target.step_into(thread_id,
                         avoid_nodebug,
                         target_func,
                         avoid_regex,
                         step_through_regex);
    };

    perform_thread_run_action(thread_id, action);
}


void core_target_impl::step_over(unsigned long thread_id,
                                 const std::string & step_through_regex) {
    auto action = [step_through_regex](auto & target, auto thread_id) {
        target.step_over(thread_id, step_through_regex);
    };

    perform_thread_run_action(thread_id, action);
}


void core_target_impl::step_out(unsigned long thread_id,
                                bool avoid_nodebug,
                                const std::string & step_through_regex) {
    auto action = [avoid_nodebug,
                   step_through_regex](auto & target, auto thread_id) {
        target.step_out(thread_id, avoid_nodebug, step_through_regex);
    };

    perform_thread_run_action(thread_id, action);
}


void core_target_impl::step_inst_into(unsigned long thread_id) {
    perform_thread_run_action(thread_id, &core::target::step_inst_into);
}



void core_target_impl::step_inst_over(unsigned long thread_id) {
    perform_thread_run_action(thread_id, &core::target::step_inst_over);
}


void core_target_impl::step_until(unsigned long thread_id, unsigned int line) {
    perform_thread_run_action(thread_id,
    [this, line](core::target & targ, unsigned long tid) {
        targ.step_until(tid, line);
    });
}


/// Extracts location info from breakpoint
static void extract_location_info(const core::target & targ,
                                  const core::code_breakpoint & bp,
                                  dbg::code_breakpoint_info & bp_info) {

    for (auto it = bp.locations_begin(), e = bp.locations_end(); it != e; ++it) {
        core::breakpoint_location loc = *it;

        dbg::breakpoint_location_info loc_inf(loc.id(),
                                              extract_code_position_info(targ, loc.pos(), loc.pos()));
        bp_info.add_location(loc_inf);
    }
}


/// Checks new breakpoint properties
void check_initial_bp_props(const core::code_breakpoint & bp) {
    assert(bp.enabled() && "breakpoint should be enabled after adding");
    assert(bp.condition() == "" && "breakpoint condition should be empty after adding");
    assert(bp.ignore_count() == 0 &&
           "breakpoint ignore count should be zero after adding");

    // current hit count is not neccessary zero in case we adding breakpoint while running
    // and it's already was hit
}


void core_target_impl::add_breakpoint(const dbg::source_position_info & pos,
                                      bool is_enabled,
                                      const std::string & condition,
                                      const code_breakpoint_handler & handl) {
    // adding new breakpoint
    core_async_queue_.add_command_eh(
    [handl](auto && bp_inf_sp) { handl(*bp_inf_sp); },
    [this, pos, is_enabled, condition] {
        // creating breakpoint in core
        auto bp = targ_->add_breakpoint(core::source_position(pos.path(), pos.line()));
        bp->set_condition(condition);
        if (!is_enabled) {
            bp->disable();
        }

        // extracting breakpoint info
        check_initial_bp_props(*bp);
        return convert_code_breakpoint(bp, *targ_);
    });
}


void core_target_impl::add_breakpoint(const std::string & func_name,
                                      bool is_enabled,
                                      const std::string & condition,
                                      const code_breakpoint_handler & handl) {
    // adding new breakpoint
    core_async_queue_.add_command_eh(
    [handl](auto && bp_inf_sp) { handl(*bp_inf_sp); },
    [this, func_name, is_enabled, condition] {
        // creating breakpoint in core
        core::code_breakpoint * bp = targ_->add_breakpoint(func_name);
        bp->set_condition(condition);
        if (!is_enabled) {
            bp->disable();
        }

        // extracting breakpoint info
        check_initial_bp_props(*bp);
        return convert_code_breakpoint(bp, *targ_);
    });
}


void core_target_impl::add_breakpoint(std::uint64_t addr,
                                      bool is_enabled,
                                      const std::string & condition,
                                      const code_breakpoint_handler & handl) {
    // adding new breakpoint
    core_async_queue_.add_command_eh(
    [handl](auto && bp_inf_sp) { handl(*bp_inf_sp); },
    [this, addr, is_enabled, condition] {
        // creating breakpoint in core
        core::code_breakpoint * bp = targ_->add_breakpoint(addr);
        bp->set_condition(condition);
        if (!is_enabled) {
            bp->disable();
        }

        // extracting breakpoint info
        check_initial_bp_props(*bp);
        return convert_code_breakpoint(bp, *targ_);
    });
}


void core_target_impl::remove_breakpoint(const breakpoint_id & id) {
    // removing breakpoint
    core_async_queue_.add_command_eh([] {},
    [this, id] {
        targ_->remove_breakpoint(targ_->get_breakpoint_by_id(id));
    });
}


void core_target_impl::enable_breakpoint(const breakpoint_id & id) {
    // enabling breakpoint
    core_async_queue_.add_command_eh([] {},
    [this, id] {
        //std::cerr << "ENABLE bp: (" << (int)id.type() << ", " << id.num() << ")\n";
        targ_->get_breakpoint_by_id(id)->enable();
    });
}


void core_target_impl::disable_breakpoint(const breakpoint_id & id) {
    // disabling breakpoint
    core_async_queue_.add_command_eh([] {},
    [this, id]() {
        targ_->get_breakpoint_by_id(id)->disable();
    });
}


void core_target_impl::set_breakpoint_condition(const breakpoint_id & id,
                                                const std::string & cond) {
    // setting breakpoint condition
    core_async_queue_.add_command_eh([] {},
    [this, id, cond]() {
        targ_->get_breakpoint_by_id(id)->set_condition(cond);
    });
}


void core_target_impl::set_breakpoint_ignore_count(const breakpoint_id & id,
                                                   unsigned int cnt) {
    // setting breakpoint ignore count
    core_async_queue_.add_command_eh([] {},
    [this, id, cnt]() {
        targ_->get_breakpoint_by_id(id)->set_ignore_count(cnt);
    });
}


void core_target_impl::add_var_watchpoint(const std::string & name,
                                          bool read,
                                          bool write,
                                          size_t size,
                                          const watchpoint_handler & handl) {
    // checking target state
    assert((state() == state_t::stopped) &&
           "target::add_var_watchpoint can be called only in stopped state");

    // adding new watchpoint
    core_async_queue_.add_command(handl,
    [this, name, read, write, size] {
        // creating breakpoint in core
        auto wp = targ_->add_var_watchpoint(name, read, write, size);
        return convert_watchpoint(wp);
    });
}


void core_target_impl::add_expr_watchpoint(const std::string & epxr,
                                           bool read,
                                           bool write,
                                           size_t size,
                                           const watchpoint_handler & handl) {
    // checking target state
    assert((state() == state_t::stopped) &&
           "target::add_expr_watchpoint can be called only in stopped state");

    // adding new watchpoint
    core_async_queue_.add_command(handl,
    [this, epxr, read, write, size] {
        // creating breakpoint in core
        auto wp = targ_->add_expr_watchpoint(epxr, read, write, size);
        return convert_watchpoint(wp);
    });
}


void core_target_impl::set_current_stack_frame(std::size_t index) {
    // checking target state
    assert(state() == state_t::stopped &&
           "set_current_stack_frame can be called only in stopped state");

    // setting current stack frame
    core_async_queue_.add_command_eh(
    [this](const auto & locals, const auto & watch) {
        locals_.emit_tree_updated(locals);
        watch_.emit_tree_updated(watch);
    },
    [this, index] {
        // setting current frame in core library
        targ_->set_current_stack_frame_index(index);

        // reading watch lists for selected thread
        return std::make_tuple(locals_.make_values_tree(), watch_.make_values_tree());
    }
    );
}


void core_target_impl::set_current_thread(std::size_t index) {
    // checking target state
    assert(state() == state_t::stopped &&
           "set_current_thread can be called only in stopped state");

    // setting current thread
    core_async_queue_.add_command_eh(
    [this](const auto & locals, const auto & watch) {
        locals_.emit_tree_updated(locals);
        watch_.emit_tree_updated(watch);
    },
    [this, index] {
        // setting current thread
        targ_->set_current_thread_index(index);

        // reading watch lists for selected thread
        return std::make_tuple(locals_.make_values_tree(), watch_.make_values_tree());
    });
}


void core_target_impl::fetch_all_frames(const thread_handler & handl) {
    // checking target state
    assert(state() == state_t::stopped &&
           "set_current_thread can be called only in stopped state");

    // setting current thread
    core_async_queue_.add_command_eh(
    handl,
    [this] {
        core::thread thrd = targ_->thread_at(targ_->current_thread_index());
        return std::make_shared<dbg::thread_info>(convert_core_thread(thrd, *targ_, true));
    });
}


void core_target_impl::set_state(state_t st,
                                 const dbg::process_context * ctx,
                                 int exit_code) {
    CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "set_state " << state_name(st);

    state_ = st;

    // TODO: refactor and move breakpoints / threads functionality to separate
    // classes; don't pass process context to implementation clients
    state_changed_signal_(st, ctx, exit_code);

    if (ctx != nullptr) {
        // updating list of locals
        locals_.emit_tree_updated(ctx->locals());

        // updating list in watch
        watch_.emit_tree_updated(ctx->watch());

        // updating registers
        registers_.emit_tree_updated(ctx->registers());
    }

    // clearing locals / watch list / registers if not in stopped/running state
    if (st == state_t::loaded ||
        st == state_t::launching ||
        st == state_t::terminating ||
        st == state_t::unloading) {
        // clearing locals
        locals_.emit_tree_updated({});

        // custom watch will build empty tree for not stopped state
        watch_.emit_tree_updated(watch_.make_values_tree());

        // clearing registers
        registers_.emit_tree_updated({});
    }
}


void core_target_impl::process_stopped() {
    CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "process stopped begin";

    // creating process context
    std::shared_ptr<dbg::process_context> ctx(new dbg::process_context);
    ctx->set_pid(targ_->pid());

    // passing stop reason to process context
    dbg::process_context::stop_reason_t st_reason = dbg::process_context::stop_reason_unknown;
    switch (targ_->stop_reason()) {
    case core::stop_reason_t::trace:
        CXXDBG_LOG_SCAT_INFO(appcore, t   arget) << "stop reason: trace";
        st_reason = dbg::process_context::stop_reason_trace;
        break;
    case core::stop_reason_t::breakpoint:
        CXXDBG_LOG_SCAT_INFO(appcore, target) << "stop reason: breakpoint";
        st_reason = dbg::process_context::stop_reason_breakpoint;
        break;
    case core::stop_reason_t::watchpoint:
        CXXDBG_LOG_SCAT_INFO(appcore, target) << "stop reason: watchpoint";
        st_reason = dbg::process_context::stop_reason_watchpoint;
        ctx->set_watchpoint_num(targ_->watchpoint_num());
        break;
    case core::stop_reason_t::signal:
        CXXDBG_LOG_SCAT_INFO(appcore, target) << "stop reason: signal";
        st_reason = dbg::process_context::stop_reason_signal;
        break;
    case core::stop_reason_t::exec:
        CXXDBG_LOG_SCAT_INFO(appcore, target) << "stop reason: exec";
        st_reason = dbg::process_context::stop_reason_exec;
        break;
    case core::stop_reason_t::crashed:
        CXXDBG_LOG_SCAT_INFO(appcore, target) << "stop reason: crashed";
        st_reason = dbg::process_context::stop_reason_crashed;
        break;
    default:
        CXXDBG_LOG_SCAT_INFO(appcore, target) << "stop reason: unknown";
        st_reason = dbg::process_context::stop_reason_unknown;
        break;
    }

    ctx->set_stop_reason(st_reason);

    // auto-selecting active thread
    targ_->select_active_thread_by_stop_reason();

    // auto-selecting active frame
    auto_select_current_frame();

    // reading modules info if stop reason is exec
    if (st_reason == dbg::process_context::stop_reason_exec) {
        ctx->set_modules(read_all_modules(*targ_));
    }

    // setting signal number if stop reason is signal
    if (st_reason == dbg::process_context::stop_reason_signal) {
        ctx->set_stop_signal(targ_->signal_num());
    }

    // converting breakpoints
    for (auto && bp : targ_->breakpoints()) {
        ctx->breakpoints().push_back(convert_breakpoint(bp, *targ_));
    }

    // converting threads
    for (auto && thrd : targ_->threads()) {
        ctx->threads().add(convert_core_thread(thrd, *targ_, false));
    }

    // building value tree for watch lists
    ctx->set_locals(locals_.make_values_tree());
    ctx->set_watch(watch_.make_values_tree());
    ctx->set_registers(registers_.make_values_tree());

    // setting active thread
    ctx->threads().set_current_thread(targ_->current_thread_index());

    // changing state in implementation
    main_thread_queue_.post([this, ctx] {
        CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "changing state from " << state_name(state()) << " to stopped";
        assert((state() == state_t::running || state() == state_t::launching) &&
               "Invalid target impl state");
        set_state(state_t::stopped, ctx.get());
    });

    CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "process stopped end";
}


void core_target_impl::on_core_state_changed(core::target::state_t st) {
    CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "received core state changed event: " << core::target::state_name(st);
    // executed in core thread

    // checking launching flag
    if (is_launching_) {
        // the process is now launching. We should ignore all change state events and only save
        // or clear is_stopped_after_launching_ flag depending on the event.
        CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "target is launching, ignoring state changed events";
        if (st == core::target::state_t::running) {
            CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "clear is_stopped_after_launching";
            is_stopped_after_launching_ = false;
        } else if (st == core::target::state_t::stopped) {
            CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "set is_stopped_after_launching";
            is_stopped_after_launching_ = true;
        }

        return;
    }

    // converting core state to application state
    switch (st) {
    case core::target::state_t::launching:
        // ignore launching state event
        CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "ignoring state change to launching";
        break;

    case core::target::state_t::stopped:
        // target may be in launching, running state
        // extracting process context and posting stop event
        CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "processing stop event";
        process_stopped();
        break;

    case core::target::state_t::running:
        // Ignoring the running state event. We don't need that becase we change state to
        // running before each action (resume, step, etc), and after launch process is finished.
        CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "ignoring state change to running";
        break;

    case core::target::state_t::loaded:
    {
        CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "processing target exit event";

        // target exited, changing state to loaded
        int ecode = targ_->exit_code();
        main_thread_queue_.post([this, ecode] {
            // target may exit only in running/terminating/detaching state
            assert((state() == state_t::running ||
                    state() == state_t::terminating ||
                    state() == state_t::detaching) &&
                   "Invalid target state");
            set_state(state_t::loaded, nullptr, ecode);
        });

        // clearing custom watch list tree
        watch_.clear();
    }
        break;

    default:
        assert(false && "Unknown core target state");
        break;
    }
}


/// Read list of sources from module and converts it to module info
static dbg::module_info read_module(const core::module & mod) {

    dbg::module_info mod_inf(mod.path(),
                             std::make_shared<core_module_impl>(mod));

    CXXDBG_APPCORE_LOG_TRACE << "loaded module " << mod.path();

    for (auto && cu : mod.compile_units()) {
        CXXDBG_APPCORE_LOG_TRACE << "loaded compile unit " << cu.source_path();

        auto src_path = !cu.source_path().empty() ? cu.source_path() : fs::path{"<empty>"};
        dbg::compile_unit_info cuinf(src_path);
        mod_inf.add_compile_unit(cuinf);

        if (!cu.source_path().empty()) {
            mod_inf.add_source(cu.source_path());
        }

        for (auto sit = cu.support_files_begin(), send = cu.support_files_end(); sit != send; ++sit) {
            CXXDBG_APPCORE_LOG_TRACE << "loaded support file " << *sit;

            // filtering out empty support files
            if (sit->empty()) {
                continue;
            }

            mod_inf.add_source(*sit);
        }
    }


    // reading function symbols

    mod_inf.funcs().reserve(static_cast<size_t>(mod.symbols().size()));

    for (const auto & sym : mod.symbols()) {
        if (!sym.is_func())
            continue;

        const char * nm = sym.name();
        if (nm == nullptr)
            continue;

        mod_inf.funcs().push_back(nm);
    }


    // sorting function symbols
    std::sort(mod_inf.funcs().begin(), mod_inf.funcs().end(), [](const char * s1, const char * s2) {
        return (strcmp(s1, s2)) < 0 ? true : false;
    });


    return mod_inf;
}


void core_target_impl::on_core_modules_loaded(const std::vector<core::module> & mods) {
    CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "modules loaded (" << mods.size() << ")";

    // building list of loaded modules, and extracting module infos

    auto mod_infos = std::make_shared<std::vector<dbg::module_info>>();
    for (auto && m : mods) {
        auto mod = m;
        if (mod_list_.count(mod.path()) > 0) {
            // existing module, skip
            continue;
        }

        fs::path mod_path = mod.path();
        main_thread_queue_.post([this, mod_path] {
            module_load_started()(mod_path);
        });

        // reindexing module
        mod.reindex();

        mod_list_.insert(mod.path());
        mod_infos->push_back(read_module(mod));

        main_thread_queue_.post([this, mod_path] {
            module_load_finished()();
        });
    }


    // sending modules loaded signal
    main_thread_queue_.post([this, mod_infos] {
        modules_loaded()(*mod_infos);
    });
}

void core_target_impl::on_core_modules_unloaded(const std::vector<core::module> & mods) {
    CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "modules unloaded (" << mods.size() << ")";
    auto mod_paths_r = mods | std::ranges::views::transform([](auto && m) { return m.path(); });
    auto mod_paths = std::make_shared<std::vector<fs::path>>(std::ranges::begin(mod_paths_r), std::ranges::end(mod_paths_r));
    main_thread_queue_.post([this, mod_paths] {
        modules_unloaded()(*mod_paths);
    });
}


void core_target_impl::auto_select_current_frame() {

    // if stop reason is signal then setting current frame to the first
    // frame that is not in system library (libc) and has debug info

    if (targ_->stop_reason() != core::stop_reason_t::signal)
        return;

    core::thread thrd = targ_->thread_at(targ_->current_thread_index());
    int cur_frame = 0;
    auto frames = thrd.frames() | std::ranges::views::common;
    for (auto it = std::ranges::begin(frames), end = std::ranges::end(frames);
         it != end && cur_frame < max_frames;
         ++it, ++cur_frame) {

        core::stack_frame frm = *it;
        // *it may return invalid frame. TODO: refactor
        if (!frm.is_valid())
            break;

        core::code_position pos = frm.read_pos();
        if (!pos) {
            break;
        }

        if (cur_frame != 0) {
            pos = pos.offset(-1);
        }

        core::module mod = pos.get_module();

        CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "auto select frame: processing function '" << pos.get_func_name()
                               << "' from module '" << mod.path() << "'";

        // skipping frames with module with no path (probably syscall)
        if (mod.path().empty()) {
            CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "auto select frame: skipping frame with no module";
            continue;
        }

        // skipping frames in libc

        auto fname = mod.path().stem();
        while (fname.stem() != fname) {
            fname = fname.stem();
        }

        if (fname == "libc") {
            CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "auto select frame: skipping libc frame";
            continue;
        }

        // skipping frames with no debug
        if (!pos.get_src_pos()) {
            CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "auto select frame: skipping frame with no source position";
            continue;
        }

        CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "auto select frame: select frame";
        thrd.set_selected_frame(cur_frame);
        break;
    }
}


void core_target_impl::perform_thread_run_action(unsigned long id, const thread_action & act) {
    // can be called only in stopped tate
    assert((state() == state_t::stopped) &&
           "target::perform_thread_run_action can be called only in stopped state");

    // changing state to running
    set_state(state_t::running);

    // executing action for specified thread
    core_async_queue_.add_command(
    [this](const async::result<void> & res) {
        // check EH error
        if(!res.is_ok()) {
            set_state(state_t::stopped);
            throw std::runtime_error(res.error());
        }
    },
    [this, id, act]() {
        act(*targ_, id);
    });
}


dbg::watch_list_impl & core_target_impl::locals_list() {
    return locals_;
}


dbg::custom_watch_list_impl &core_target_impl::watch_list() {
    return watch_;
}


dbg::watch_list_impl & core_target_impl::registers_list() {
    return registers_;
}


void core_target_impl::send_stdin(const std::string & data) {
    assert((state() == state_t::running || state() == state_t::stopped) &&
           "can't send stdin to not running target");

    core_async_queue_.add_command(
    [this](const async::result<void> & res) {
        if (!res.is_ok()) {
            throw std::runtime_error{res.error()};
        }
    },
    [this, data] {
        targ_->send_stdin(data);
    });
}


void core_target_impl::read_memory(uint64_t addr,
                                   size_t size,
                                   const async::result_handler<std::vector<char>> & h) {
    assert(state() == state_t::stopped && "can read memory only in stopped state");

    core_async_queue_.add_command(
    h,
    [this, addr, size] {
        return targ_->read_memory(addr, size);
    }
    );
}


std::shared_ptr<std::vector<dbg::module_info>>
core_target_impl::read_all_modules(const core::target & targ) {

    std::shared_ptr<std::vector<dbg::module_info>> mods(new std::vector<dbg::module_info>);

    for (auto it = targ.modules_begin(), e = targ.modules_end(); it != e; ++it) {
        mods->push_back(read_module(*it));
    }

    return mods;
}

}
