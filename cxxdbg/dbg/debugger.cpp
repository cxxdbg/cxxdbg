// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file debugger.cpp
/// Contains implementation of debugger class.

#include "debugger.hpp"
#include "breakpoint_list.hpp"
#include "breakpoint_utils.hpp"
#include "breakpoints_view_model.hpp"
#include "call_stack_model.hpp"
#include "custom_watch_list.hpp"
#include "custom_watch_list_impl.hpp"
#include "dbg_terminal.hpp"
#include "debugger_impl.hpp"
#include "debugger_ui.hpp"
#include "log.hpp"
#include "module_info.hpp"
#include "platform.hpp"
#include "cxxdbg/dbg/breakpoints_view_model.hpp"
#include "source_model.hpp"
#include "stop_reason.hpp"
#include "target.hpp"
#include "thread_utils.hpp"
#include "watchpoint.hpp"
#include "cxxdbg/dbg/core/regex_lists.hpp"
#include "dbgfmt/format_options.hpp"
#include "cxxdbg/util/convert.hpp"
#include "cxxdbg/util/string_vector.hpp"

#include <cassert>
#include <functional>
#include <memory>


namespace fs = std::filesystem;


namespace cxxdbg::dbg {


static std::vector<std::string> state_names = {
    "initial",
    "connecting",
    "loading",
    "loaded",
    "unloading",
    "launching",
    "running",
    "stopped",
    "terminating",
    "detaching"
};


debugger::debugger(settings_storage & sett,
                   async::event_queue & main_thrd_queue,
                   source_model & src_model,
                   const debugger_impl_sp & impl,
                   debugger_ui & ui):
settings_{sett},
platforms_{std::make_unique<platform_list>()},
state_{state_t::initial},
main_thread_queue_(main_thrd_queue),
src_model_(src_model),
impl_(impl),
ui_{ui},
breakpoints_{std::make_unique<breakpoint_list>(src_model_)},
breakpoints_model_{std::make_unique<breakpoints_view_model>(*breakpoints_)},
exe_term_{std::make_unique<dbg_terminal>()},
locals_{std::make_unique<watch_list>(nullptr)},
watch_{std::make_unique<custom_watch_list>(nullptr)},
registers_{std::make_unique<watch_list>(nullptr)} {

    init_term_settings();

    platform_ = platforms().host();
    sel_platform_ = platform_;

    cs_mdl_ = std::make_unique<call_stack_model>();

    // loading and applying settings
    debug_settings new_settings;
    new_settings.load_settings(settings_);
    set_debug_sett(new_settings);
}


debugger::~debugger() {
}


const std::string & debugger::state_name(state_t st) {
    auto idx = static_cast<size_t>(st);
    assert(idx < state_names.size() && "invalid state_value");
    return state_names[idx];
}


const std::string & debugger::state_name() const {
    return state_name(state());
}


void debugger::set_state(debugger::state_t s) {
    CXXDBG_LOG_SCAT_DEBUG(dbg, debugger) << "set_state " << state_name(s);
    state_ = s;
    state_changed()();
}


void debugger::execute(const std::string & cmd, const execute_result_handler & handler) {
    exec_cmds({cmd}, handler);
}


void debugger::exec_cmds(const std::vector<std::string> & cmds, const execute_result_handler & handler) {
    impl_->exec_cmds(cmds, handler);
}


void debugger::set_settings_value(const std::string & name,
                                  const std::string & val,
                                  const async::result_handler<> & handler) {
    impl_->set_settings_value(name, val, handler);
}


void debugger::set_settings_list_value(const std::string & name,
                                       const std::vector<std::string> & values,
                                       const async::result_handler<> & handler) {
    set_settings_value(name, util::string_vector_to_string_spaces(values), handler);
}


void debugger::set_debug_sett(const debug_settings & sett) {

    // setting step avoid regex options
    if (sett.regexp_skip_lists() != debug_sett().regexp_skip_lists()) {
        std::vector<std::string> step_avoid_list;
        for (auto && list : sett.regexp_skip_lists()) {
            if (!list.enabled()) {
                continue;
            }

            for (auto && regex : list.data()) {
                step_avoid_list.push_back(regex);
            }
        }

        set_settings_list_value("target.process.thread.step-avoid-regexp", step_avoid_list);
    }

    // setting step through regex options
    if (sett.step_through_lists() != debug_sett().step_through_lists()) {
        std::vector<std::string> step_through_list;
        for (auto && list : sett.step_through_lists()) {
            if (!list.enabled()) {
                continue;
            }

            for (auto && regex : list.data()) {
                step_through_list.push_back(regex);
            }
        }

        set_settings_list_value("target.process.thread.step-through-regexp", step_through_list);
    }


    bool should_notify_gf_changed = !(debug_sett().group_functions_lists() == sett.group_functions_lists());
    bool f_opts_changed = debug_sett().fmt_opts() != sett.fmt_opts();

    debug_sett_ = sett;
    debug_sett_.save_settings(settings_);

    // notifying call stack model about changing list of function to group
    if (should_notify_gf_changed) {
        cs_mdl_->group_functions_lists_changed(debug_sett().group_functions_lists().combine_regex());
    }

    if (f_opts_changed) {
        // notifying listeners about changing format options
        fmt_opts_changed()();

        // setting format options in watch lists
        locals_->set_fmt_opts(debug_sett().fmt_opts());
        watch_->set_fmt_opts(debug_sett().fmt_opts());
        registers_->set_fmt_opts(debug_sett().fmt_opts());
    }
}


void debugger::set_term_sett(const term_settings & sett) {
    if (term_sett_.tkind() != sett.tkind()) {
        term_sett_.set_tkind(sett.tkind());
        settings_.write<int>("term/kind", static_cast<int>(sett.tkind()));
    }

    if (term_sett_.xterm_path() != sett.xterm_path()) {
        term_sett_.set_xterm_path(sett.xterm_path());
        settings_.write<fs::path>("term/xterm_path", sett.xterm_path());
    }

    if (term_sett_.close_xterm() != sett.close_xterm()) {
        term_sett_.set_close_xerm(sett.close_xterm());
        settings_.write<bool>("term/close_xterm", sett.close_xterm());
    }

    if (term_sett_.custom_term_cmd() != sett.custom_term_cmd()) {
        term_sett_.set_custom_term_cmd(sett.custom_term_cmd());
        settings_.write<std::string>("term/custom_term_cmd", sett.custom_term_cmd());
    }
}


std::wstring debugger::status_text() const {

    switch (state()) {
        case state_t::initial:
            return L"Ready";
        case state_t::connecting:
            return L"Connecting to platform...";
        case state_t::loading:
            return L"Loading executable...";
        case state_t::loaded:
            return L"Loaded executable";
        case state_t::unloading:
            return L"Unloading executable...";
        case state_t::launching:
            return L"Launching executable...";
        case state_t::running:
            return L"Process is running";

        case state_t::stopped: {
            const stop_reason & rsn = current_target().stop_rsn();
            if (rsn.is_trace()) {
                return L"Process is stopped after tracing";
            } else if (rsn.is_wpoint()) {
                auto * wp_rsn = dynamic_cast<const stop_reason_watchpoint *>(&rsn);
                auto num = wp_rsn->watchpoint_num();
                auto id = core::breakpoint_id{core::breakpoint_type::watch, num};
                const breakpoint * bp = breakpoints().find_breakpoint(id);
                const watchpoint * wp = dynamic_cast<const watchpoint*>(bp);
                assert(wp != nullptr && "breakpoint is not watchpoint");
                std::wstring text =
                        L"Process is stopped at watchpoint for <" +
                                util::convert::to_wstring(wp->name()) +
                                L">. Old value: <" + wp->old_val() +
                                L">. New value: <" + wp->new_val() + L">.";
                return text;
            } else if (rsn.is_break()) {
                return L"Process is stopped at breakpoint";
            } else if (rsn.is_signal()) {
                auto sig_rsn = dynamic_cast<const stop_reason_signal *>(&rsn);
                assert(sig_rsn && "stop reason is not signal");
                std::ostringstream msg;
                msg << "Process is stopped on signal " << sig_rsn->sig_name();
                return util::convert::to_wstring(msg.str());
            } else if (rsn.is_exec()) {
                return L"Process is stopped on exec";
            } else if (rsn.is_crash()) {
                return L"Process is crashed";
            } else {
                return L"Process is stopped by unknown reason";
            }
        }
        case state_t::terminating:
            return L"Process is terminating...";
        case state_t::detaching:
            return L"Detaching from process...";
        default:
            assert(false && "unknown application state");
            return {};
    }
}


void debugger::connect_to_platform(const platform_connection_options & opts,
                                   const connect_to_platform_handler & handler) {
    assert(opts.plat != nullptr && "platform should not be null");
    assert(state() == state_t::initial && "invalid state for connecting to platform");

    CXXDBG_DBG_LOG_DEBUG << "connecting to target " << opts.plat->name() << " with url: '" << opts.url << "'";
    set_state(state_t::connecting);
    impl_->connect_to_platform(opts, [this, handler, p = opts.plat](auto && res) {
        if (res.is_ok()) {
            CXXDBG_DBG_LOG_DEBUG << "connected to platform";
            platform_ = p;
            connected_platform_changed();
        } else {
            CXXDBG_DBG_LOG_DEBUG << "error_connecting to platform: " << res.error();
        }

        set_state(state_t::initial);
        handler(res);
    });
}


void debugger::disconnect_from_platform() {
    assert(state() == state_t::initial && "invalid state for disconnecting from platform");
    platform_connection_options opts;
    opts.plat = platforms_->host();
    impl_->connect_to_platform(opts, [](auto && res) {});
    platform_ = platforms_->host();
    connected_platform_changed();
}


void debugger::load_target(const std::filesystem::path & exe_path,
                           const async::result_handler<> & handler) {

    assert(state() == state_t::initial && "Invalid application state");

    // change debugger state to loading
    set_state(state_t::loading);

    impl_->load_target(exe_path,
    [this, exe_path, handler](const async::result<debugger_impl::target_load_result> & res) {
        CXXDBG_DBG_LOG_DEBUG << "loading target finished";

        // checking for EH error
        if (!res.is_ok()) {
            CXXDBG_DBG_LOG_ERROR << "error while loading target: " << res.error();
            set_state(state_t::initial);
            handler(async::result<target_sp>(res.error()));
            return;
        }

        // creating target object
        debugger_impl::target_load_result lres = res.value();
        target_sp targ(new target(*this,
                                  main_thread_queue_,
                                  lres.targ_impl,
                                  src_model_,
                                  *breakpoints_,
                                  *lres.modules,
                                  exe_path));

        // converting main pos
        source_position src_pos;
        if (lres.main_pos) {
            source_file * src = src_model_.source(lres.main_pos.path());
            src_pos = source_position(src, lres.main_pos.line());
        }

        // finishing loading of target
        on_target_load_attach_complete(async::ok_result(targ), *lres.modules, src_pos);

        // invoking handler
        handler(async::result<void>{});
    });
}


void debugger::attach(const std::variant<unsigned long, std::string> & targ,
                      const async::result_handler<> & handler) {
    assert((state() == state_t::initial || state() == state_t::loaded) &&
           "Invalid application state");

    // resetting breakpoints
    breakpoints().reset_breakpoints();

    if (has_target()) {
        // target already loaded, attaching using existing target
        current_target().attach(targ, handler);
    } else {
        // target is not loaded, asks debugger to create new target
        // and attach

        // change application state to state_t::loading
        set_state(state_t::loading);

        impl_->attach(targ,
        [this, handler](const async::result<debugger_impl::target_load_result> & res) {
            // checking for EH error
            if (!res.is_ok()) {
                set_state(state_t::initial);
                handler(res.to<void>());
                return;
            }

            // creating target object
            debugger_impl::target_load_result lres = res.value();
            target_sp targ(new target(*this,
                                      main_thread_queue_,
                                      lres.targ_impl,
                                      src_model_,
                                      *breakpoints_,
                                      *lres.modules));

            // converting main pos
            source_position src_pos;
            if (lres.main_pos) {
                source_file * src = src_model_.source(lres.main_pos.path());
                src_pos = source_position(src, lres.main_pos.line());
            }

            // completing loading target
            on_target_load_attach_complete(async::ok_result(targ), *lres.modules, src_pos);

            // invoking handler
            handler(async::result<>{});
        });
    }
}


bool debugger::has_target() const {
    switch (state()) {
    case state_t::initial:
    case state_t::loading:
    case state_t::unloading:
        return false;

    case state_t::loaded:
    case state_t::launching:
    case state_t::running:
    case state_t::stopped:
    case state_t::terminating:
    case state_t::detaching:
        return true;

    default:
        assert(false && "Unknown debugger state");
        return false;
    }
}


bool debugger::has_process() const {
    return state() == state_t::stopped || state() == state_t::running;
}


target & debugger::current_target() {
    assert(has_target() && "no current target");
    return *targ_;
}


const target & debugger::current_target() const {
    assert(has_target() && "no current target");
    return *targ_;
}


void debugger::close_target(const async::result_handler<void> & handler) {
    assert(state() == state_t::loaded &&
           "close_target can be called only in loaded state");

    // setting state to unloading
    set_state(state_t::unloading);

    // start closing target
    targ_->close([this, handler]() {
        // setting state to initial
        set_state(state_t::initial);

        // disconnecting breakpoint list
        breakpoints().disconnect_impl();

        // removing target from locals watch
        locals_->set_impl(nullptr);

        // removing target from custom watch
        watch_->set_impl(nullptr);

        // removing target from registers list
        registers_->set_impl(nullptr);

        // disconnecting executable terminal from target
        exe_term_->disconnect();

        // removing target
        set_target({});

        handler(async::result<>{});
    });
}


/// Helper class for fetching launch result and skipping stop events
class launch_helper {
public:
    typedef std::function<void()> completion_handler;

    launch_helper(debugger & app, bool in_term) :
    dbg_{app},
    num_stops_to_skip_{in_term ? 2 : 0U} {
        CXXDBG_DBG_LOG_DEBUG << "launch helper: initialized with skip stopts = " << num_stops_to_skip_;
        state_changed_con_ = dbg_.state_changed().connect([this] { on_state_changed(); });
    }


    ~launch_helper() {
        CXXDBG_DBG_LOG_DEBUG << "launch helper: destroyed";
    }


    void set_comp_handler(const completion_handler & h) {
        handler_ = h;
    }

    /// State changed event handler
    void on_state_changed() {
        CXXDBG_DBG_LOG_DEBUG << "launch helper: on state changed: " << dbg_.state_name(dbg_.state());

        if (dbg_.state() == debugger::state_t::stopped) {
            CXXDBG_DBG_LOG_DEBUG << "launch helper: state changed to stopped, skip stop";

            // clearing thread list after temporary stops
            dbg_.current_target().clear_threads();

            // skipping stop
            --num_stops_to_skip_;
            dbg_.run();

            if (num_stops_to_skip_ == 0 && result_received_) {
                // result received and all required stops skipped,
                // notifying user
                CXXDBG_DBG_LOG_DEBUG << "launch helper: invoke handler";
                handler_();
            }
        }
    }

    /// Launch completion handler
    void on_launched(const async::result<unsigned long> & res) {
        CXXDBG_DBG_LOG_DEBUG << "launch helper: launch result received";

        // saving result
        res_ = res;
        result_received_ = true;

        if (!res_.is_ok()) {
            // launch failed, notifying user
            CXXDBG_DBG_LOG_DEBUG << "launch helper: launch failed, invoke handler";
            handler_();
            return;
        }

        if (num_stops_to_skip_ == 0) {
            // all stops already skipped, notifying user
            CXXDBG_DBG_LOG_DEBUG << "launch helper: invoke handler";
            handler_();
        }
    }

    /// Returns launch result
    async::result<unsigned long> result() const {
        return res_;
    }

private:
    debugger & dbg_;
    completion_handler handler_;
    scoped_signal_connection state_changed_con_;
    unsigned int num_stops_to_skip_;
    async::result<unsigned long> res_{"unknown error"};
    bool result_received_ = false;
};


void debugger::launch(const launch_options & lopts,
                      const async::result_handler<unsigned long> & handler) {

    assert(state() == state_t::loaded &&
           "application::launch can be called only in loaded state");

    bool in_terminal = (term_sett_.tkind() != term_settings::term_kind::builtin);

    // disable launch in terminal for not host targets
    if (connected_platform() != platforms().host()) {
        in_terminal = false;
    }

    // creating helper object for handling temporary stops when launching in terminal
    auto helper = new launch_helper{*this, in_terminal};
    helper->set_comp_handler([helper, handler] {
        // we must delete helper after invoking handler because all
        // captured variables and this lambda will be destroyes in helper destructor
        handler(helper->result());
        delete helper;
    });

    // resetting all breakpoints
    breakpoints().reset_breakpoints();

    // launching application
    auto compl_handler = [helper](const auto & res) { helper->on_launched(res); };
    if (in_terminal) {
        current_target().launch_terminal(lopts, term_sett_, compl_handler);
    } else {
        current_target().launch(lopts, compl_handler);
    }

    assert(state() == state_t::launching &&
           "Invalid application state after starting launching");
}


void debugger::run() {
    assert(state() == state_t::stopped &&
           "application::run can be called only in stopped state");

    current_target().run();
}


void debugger::stop() {
    assert((state() == state_t::stopped || state() == state_t::running) &&
           "application::stop can be called only in stopped or running state");

    current_target().stop();
}


void debugger::terminate() {
    assert(state() == state_t::stopped &&
           "application::terminate can be called only in stopped state");

    current_target().terminate();
}


void debugger::detach() {
    assert(state() == state_t::stopped &&
           "application::detach can be called only in stopped state");

    current_target().detach();
}


void debugger::step_into(boost::logic::tribool avoid_nodebug,
                         const std::string & target,
                         const std::string & avoid_regex,
                         const std::string & step_through_regex) {

    assert(state() == state_t::stopped &&
           "application::step_into can be called only in stopped state");

    auto b_avoid_nodebug = false;
    if (boost::indeterminate(avoid_nodebug)) {
        b_avoid_nodebug = debug_sett_.is_avoid_nodebug();
    } else {
        b_avoid_nodebug = static_cast<bool>(avoid_nodebug);
    }

    current_target().step_into(b_avoid_nodebug,
                               target,
                               avoid_regex,
                               step_through_regex);
}


void debugger::step_into_call_target(const std::string & target_name) {
    auto avoid_nodebug = boost::indeterminate;
    step_into(avoid_nodebug, target_name);
}


void debugger::step_over(const std::string & step_through_regex) {
    assert(state() == state_t::stopped &&
           "application::step_over can be called only in stopped state");

    current_target().step_over(step_through_regex);
}


void debugger::step_out(boost::tribool avoid_nodebug, const std::string & step_through_regex) {
    assert(state() == state_t::stopped &&
           "application::step_out can be called only in stopped state");

    auto b_avoid_nodebug = false;
    if (boost::indeterminate(avoid_nodebug)) {
        b_avoid_nodebug = debug_sett_.is_avoid_nodebug();
    } else {
        b_avoid_nodebug = static_cast<bool>(avoid_nodebug);
    }

    current_target().step_out(b_avoid_nodebug, step_through_regex);
}


void debugger::step_inst_into() {
    assert(state() == state_t::stopped &&
           "application::step_inst_into can be called only in stopped state");

    current_target().step_inst_into();
}


void debugger::step_inst_over() {
    assert(state() == state_t::stopped &&
           "application::step_inst_over can be called only in stopped state");

    current_target().step_inst_over();
}


void debugger::step_until(unsigned int line) {
    assert(state() == state_t::stopped &&
           "application::do_step_until can be called only in stopped state");

    // performing step until on current target
    current_target().step_until(line);
}


void debugger::ask_line_number_and_step_until() {
    // asking line number and checking that user clicked ok
    unsigned int line_num = 0;
    if (!ui_.ask_step_until_line_number(line_num)) {
        return;
    }

    // performing step until on current target
    step_until(line_num);
}


signal_connection debugger::connect_on_event(const event_handler & h) {
    return impl_->connect_on_event(h);
}


void debugger::notify_module_load_start(const std::filesystem::path & mod) {
    ui_.notify_module_load_start(mod);
}


void debugger::notify_module_load_finish() {
    ui_.notify_module_load_finish();
}


void debugger::show_info_message(const std::wstring & msg) {
    ui_.show_info_message(msg);
}


const thread_list & debugger::threads() const {
    return current_target().threads();
}


const thread * debugger::current_thread() const {
    if (!has_target())
        return nullptr;

    return current_target().current_thread();
}


void debugger::set_current_thread(const thread * thrd) {
    assert(state() == state_t::stopped && "set_current_thread can be called only in stopped state");
    current_target().set_current_thread(thrd);
}


const stack_frame * debugger::current_frame() const {
    if (!has_target())
        return nullptr;

    return current_target().current_frame();
}


void debugger::set_current_frame(const stack_frame * frame) {
    assert(state() == state_t::stopped &&
           "set_current_frame can be called only in stopped state");

    current_target().set_current_frame(frame);
}


void debugger::fetch_all_stack_frames() {
    assert(state() == state_t::stopped &&
           "fetch_all_stack_frames can be called only in stopped state");

    current_target().fetch_all_stack_frames();
}


std::multimap<unsigned int, const thread *>
debugger::source_threads(const source_file * src) const {
    std::multimap<unsigned int, const thread*> res;

    if (state() != debugger::state_t::stopped) {
        // can't examine threads in not stopped state
        return res;
    }

    for (auto it = threads().begin(), end = threads().end(); it != end; ++it) {
        if (std::ranges::empty((*it)->call_stack())) {
            // no call stack info in thread
            continue;
        }

        const auto & c_stack = (*it)->call_stack();
        const stack_frame * frame = *std::ranges::begin(c_stack);

        if (!frame->pos().src_pos()) {
            // no source position is available
            continue;
        }

        if (frame->pos().src_pos().file() == src) {
            res.insert(std::make_pair(frame->pos().src_pos().line(), *it));
        }
    }

    return res;
}


std::list<const thread *> debugger::line_threads(const source_file * src,
                                                 unsigned int line) const {
    std::list<const thread*> res;

    std::multimap<unsigned int, const thread*> thrds = source_threads(src);
    for (auto it = thrds.find(line), end = thrds.end();
         it != end && it->first == line;
         ++it) {
        res.push_back(it->second);
    }

    return res;
}


source_position debugger::curr_thread_pos() const {
    // can get current position only in stopped state
    if (state() != state_t::stopped) {
        return {};
    }

    const thread * thrd = current_thread();

    // checking that current thread is available
    if (thrd == nullptr) {
        return {};
    }

    // checking that call stack is available
    if (std::ranges::empty(thrd->call_stack())) {
        return {};
    }

    auto cs = thrd->call_stack();
    auto frame = *std::ranges::begin(cs);
    return frame->pos().src_pos();
}


source_position debugger::curr_frame_pos() const {
    // can get current frame position only in stopped state
    if (state() != state_t::stopped) {
        return {};
    }

    const stack_frame * frame = current_frame();

    // checking that current frame is available
    if (frame == nullptr) {
        return {};
    }

    return frame->pos().src_pos();
}


const dbgfmt::format_options & debugger::fmt_opts() const {
    return debug_sett_.fmt_opts();
}


void debugger::set_fmt_opts(const dbgfmt::format_options & opts) {
    auto dsett = debug_sett();
    dsett.set_fmt_opts(opts);
    set_debug_sett(dsett);
}


bool debugger::fmt_hex() const {
    return fmt_opts().hex();
}


void debugger::set_fmt_hex(bool val) {
    auto opts = fmt_opts();
    opts.set_hex(val);
    set_fmt_opts(opts);
}


bool debugger::fmt_ptr_addr() const {
    return fmt_opts().show_ptr_addr();
}


void debugger::set_fmt_ptr_addr(bool val) {
    auto opts = fmt_opts();
    opts.set_show_ptr_addr(val);
    set_fmt_opts(opts);
}


bool debugger::fmt_rec_addr() const {
    return fmt_opts().show_rec_addr();
}


void debugger::set_fmt_rec_addr(bool val) {
    auto opts = fmt_opts();
    opts.set_show_rec_addr(val);
    set_fmt_opts(opts);
}


std::string debugger::line_description(const source_position & pos, bool show_threads) const {
    std::ostringstream str;

    assert(pos.is_valid() && "source position should be valid here");

    // current position description
    if (pos == curr_thread_pos()) {
        str << "The next statement that will be executed";
        return str.str();
    }


    // current frame description
    if (pos == curr_frame_pos()) {
        str << "The next statement to execute when thread returns from current function";
        return str.str();
    }


    // breakpoints info
    auto bps = breakpoints_at_line(breakpoints(), pos);
    bool first = true;
    for (const breakpoint * bp : bps) {

        auto cbp = dynamic_cast<const code_breakpoint*>(bp);
        assert(cbp && "breakpoint is not a code breakpoint");

        if (first) {
            first = false;
        } else {
            str << "\n";
        }

        str << "Breakpoint at " << bp->name();
        if (cbp->locations_size() == 1) {
            const breakpoint_location * loc = *std::begin(cbp->locations());
            str << " (" << loc->pos().src_pos_str() << ")";
        } else {
            str << " (multiple locations)";
        }
    }

    if (!bps.empty()) {
        return str.str();
    }

    if (!show_threads) {
        return std::string();
    }


    // threads info
    auto thrds = threads_at_line(*this, pos);
    first = true;
    for (const thread * thrd : thrds) {

        if (first) {
            first = false;
        } else {
            str << "\n";
        }

        str << "Thread " << thrd->id();
    }

    return str.str();
}


void debugger::init_term_settings() {
    auto tkind = static_cast<term_settings::term_kind>(
            settings_.read<int>("term/kind",
                                static_cast<int>(term_settings::term_kind::builtin)));
    term_sett_.set_tkind(tkind);
    term_sett_.set_xterm_path(settings_.read<fs::path>("term/xterm_path", "/usr/bin/xterm"));
    term_sett_.set_close_xerm(settings_.read<bool>("term/close_xterm", true));
    term_sett_.set_custom_term_cmd(settings_.read<std::string>("term/custom_term_cmd", ""));
}


void debugger::on_target_load_attach_complete(const async::result<target_sp> & res,
                                              const std::vector<module_info> & mods,
                                              const source_position & main_pos) {
    CXXDBG_LOG_SCAT_DEBUG(dbg, debugger) << "on_target_load_attach_complete";

    // state should be state_t::loading
    assert(state() == state_t::loading && "Invalid application state");

    // check error
    if (!res.is_ok()) {
        // change state to initial
        CXXDBG_LOG_SCAT_DEBUG(dbg, debugger) << "target load failed, changing state to initial";
        set_state(state_t::initial);
        return;
    }

    assert(res.value().get() && "Null target");
    set_target(res.value());

    CXXDBG_LOG_SCAT_DEBUG(dbg, debugger) << "target load complete, setting state according to target state";

    // change application state according to target state
    switch (targ_->state()) {
    case target::state_t::loaded:
        // target was loaded
        set_state(state_t::loaded);
        break;
    case target::state_t::launching:
        // target is still attaching
        set_state(state_t::launching);
        break;
    case target::state_t::stopped:
        // target was attached
        set_state(state_t::stopped);
        break;
    default:
        assert(false && "Unknown target state after loading or attaching");
        break;
    }

    // connecting to target current thread changed signal
    current_target().connect_current_thread_changed([this]() {
        cs_mdl_->set_thread(current_thread());
        current_thread_changed()();
    });

    // connecting to target current stack frame changed signal
    current_target().connect_current_stack_frame_changed([this]() {
        current_frame_changed()();
    });

    // connecting breakpoint list to target
    breakpoints().connect_impl(&current_target().impl());

    // setting impl for locals watch
    locals_->set_impl(&current_target().locals());

    // setting impl for custom watch
    watch_->set_impl(&current_target().watch());

    // setting impl for registers watch
    registers_->set_impl(&current_target().registers());

    // connecting terminal to target
    exe_term_->connect(&current_target());

    // connect to target state changed signal
    current_target().connect_state_changed([this]() {
        on_target_state_changed();
    });

    // emitting target changed signal
    target_changed()(res, mods, main_pos);
}


void debugger::on_target_state_changed() {
    // application can't be in initial and loading states
    assert(state() != state_t::initial &&
           state() != state_t::loading &&
           "Invalid application state");

    // changing debugger state according to target state
    switch (targ_->state()) {
    case target::state_t::loaded:
        if (target_was_running_) {
            process_exited()(current_target().exit_code());
        }
        set_state(state_t::loaded);
        break;
    case target::state_t::launching:
        set_state(state_t::launching);
        break;
    case target::state_t::running:
        target_was_running_ = true;
        set_state(state_t::running);
        break;
    case target::state_t::stopped: {
        target_was_running_ = true;
        auto & new_call_targets = current_target().get_call_targets();
        if (call_targets() != new_call_targets) {
            call_targets() = new_call_targets;
            call_targets_changed()();
        }

        // show message if stopped at wp
        if (auto sr_wp = dynamic_cast<const stop_reason_watchpoint*>(&targ_->stop_rsn())) {
            auto * wp = breakpoints().find_watchpoint(sr_wp->watchpoint_num());
            assert(wp != nullptr);
            std::wstring msg = L"Process is stopped at watchpoint for <" +
                               util::convert::to_wstring(wp->name()) +
                               L">.\nOld value: <" + wp->old_val() +
                               L">.\nNew value: <" + wp->new_val() + L">.";
            show_info_message(msg);
        }

        set_state(state_t::stopped);
    }
        break;
    case target::state_t::terminating:
        set_state(state_t::terminating);
        break;
    case target::state_t::detaching:
        target_was_running_ = false;
        set_state(state_t::detaching);
        break;
    case target::state_t::invalid:
        // target was unloaded, ignore event.
        // we delete target in close completion handler
        break;
    case target::state_t::unloading:
        set_state(state_t::unloading);
        target_changed()(async::result<target_sp>{"no target"},
                         std::vector<module_info>{},
                         source_position{});
        break;
    default:
        assert(false && "Unknown target state");
    }
}


}
