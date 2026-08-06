// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file application.cpp
/// Contains implementation of application class.

#include "application.hpp"
#include "breakpoint_list.hpp"
#include "breakpoint_location.hpp"
#include "breakpoint_site.hpp"
#include "code_breakpoint.hpp"
#include "command_line.hpp"
#include "composite_source_tree_model.hpp"
#include "dbg_cli_processor.hpp"
#include "debugger.hpp"
#include "log.hpp"
#include "module_info.hpp"
#include "platform.hpp"
#include "progress_dialog.hpp"
#include "source_file.hpp"
#include "source_tree.hpp"
#include "source_tree_builder.hpp"
#include "stack_frame.hpp"
#include "stop_reason.hpp"
#include "target.hpp"
#include "watchpoint.hpp"

#include "cxxdbg/app/settings_storage.hpp"
#include "cxxdbg/app/text_file_document.hpp"
#include "cxxdbg/cli/app_command_interpreter.hpp"
#include "cxxdbg/util/convert.hpp"

#include <ranges.hpp>
#include <memory>
#include <sstream>


namespace fs = std::filesystem;


namespace cxxdbg::dbg {


application::application(settings_storage & sett,
                         async::event_queue & main_thrd_queue,
                         const debugger_impl_sp & dbg_impl,
                         document_list_ui & doc_ui):
settings_(sett),
documents_{doc_ui, sett, "recent_files", true},
src_tree_{std::make_unique<source_tree>()},
src_tree_mdl_{std::make_unique<composite_source_tree_model>(*src_tree_)},
dbg_(std::make_unique<debugger>(sett, main_thrd_queue, sources_, dbg_impl, *this)),
cli_proc_{new dbg_cli_processor{*this}},
interp_{new cli::app_command_interpreter{*cli_proc_}},
term_{settings_, *interp_} {

    // listening for debugger state changes
    dbg_->state_changed().connect([this] {
        state_changed_signal_();
    });

    dbg_->target_changed().connect([this](auto && res, auto && mods, auto && mpos) {
        on_target_changed(res, mods, mpos);
    });

    // changing current source position when current stack frame is changed
    connect_current_stack_frame_changed([this]() {
        auto pos = dbg_->curr_frame_pos();
        if (pos && fs::exists(pos.file()->path())) {
            show_pos(pos);
        }
    });


    // // connecting to debugger breakpoint signals

    // dbg_->breakpoints().added().connect([this](auto && bp) {
    //     this->on_breakpoint_added(bp);
    // });

    // dbg_->breakpoints().removed().connect([this](auto && bp) {
    //     breakpoint_removed_signal_(bp);
    // });

    // dbg_->breakpoints().changed().connect([this](auto && bp) {
    //     this->on_breakpoint_changed(bp);
    //     breakpoint_changed_signal_(bp);
    // });

    // dbg_->breakpoints().list_changed().connect([this]() { breakpoints_changed_signal_(); });

    // breakpoint_selection_changed_connection_ = connect_breakpoint_changed([&](auto && args) {
    //     breakpoints_selection_changed_signal_();
    // });


    // listening for changing position of toggled breakpoint
    dbg_->breakpoints().toggle_pos_changed().connect([this](auto && pos) {
        show_pos(pos);
    });
}


application::~application() {
}


application::state_t application::state() const {
    return dbg_->state();
}


bool application::has_process() const {
    return dbg_->has_process();
}


void application::process_command_line(const command_line & cmd_line) {
    // executing initialization commands
    for (auto && cmd : cmd_line.init_commands()) {
        dbg_term().exec(cmd);
    }

    // setting executable search paths
    if (!cmd_line.exec_search_paths().empty()) {
        std::ostringstream paths_str;
        bool first = true;
        for (auto && p : cmd_line.exec_search_paths()) {
            if (!first) {
                paths_str << ' ';
            } else {
                first = false;
            }

            paths_str << '"' << p << '"';
        }

        dbg_->set_settings_value("target.exec-search-paths", paths_str.str());
    }

    // connect to platform if specified in command line
    auto p_name = cmd_line.platform_name();
    if (!p_name.empty()) {
        auto p_list = dbg_->platforms().all();

        // looking for platform with specified name
        auto pit = std::ranges::find_if(p_list, [&p_name](auto && p) {
            return p->name() == p_name;
        });

        if (pit == std::ranges::end(p_list)) {
            std::ostringstream msg;
            msg << "can't find platform with name: " << p_name;
            throw std::runtime_error{msg.str()};
        }

        platform_connection_options opts;
        opts.plat = *pit;
        opts.url = cmd_line.platform_url();
        ask_and_connect(opts);
    }

    // loading executable if specified
    if (!cmd_line.exe_name().empty()) {
        launch_options opts;
        opts.set_work_dir(fs::current_path());
        opts.launch_args() = cmd_line.exe_args();
        load_target(cmd_line.exe_name(), opts);
    }
}


void application::exec_cmd(const std::string & cmd,
                           const std::function<void(const std::string &)> & handler) {
    dbg_->execute(cmd, handler);
}


void application::exec_cmds(const std::vector<std::string> & cmds,
                            const std::function<void (const std::string &)> & handler) {
    dbg_->exec_cmds(cmds, handler);
}


bool application::ask_and_connect(const platform_connection_options & opts) {
    assert(opts.plat != nullptr && "null platform in ask_and_connect");

    // asking and closing loaded/launched targets
    if (!ask_and_close()) {
        return false;
    }

    assert(state() == state_t::initial && "invalid state after ask_and_close");

    // creating progress dialog
    std::ostringstream status;
    status << "Connecting to platform " << opts.plat->name() << " with URL " << opts.url;
    auto dlg = make_progress_dialog("Connecting to platform", status.str());

    platform_connection_options real_opts = opts;

    // appending "connect://" before url if it does not contain URL scheme
    if (real_opts.url.find("://") == std::string::npos) {
        real_opts.url = "connect://";
        real_opts.url += opts.url;
    }

    // starting connecting to platform
    async::result<> res;
    dbg_->connect_to_platform(real_opts, [this, dlg, &res](auto && r) {
        // saving result
        res = r;

        // closing progress dialog
        dlg->close();
    });

    // displaying progress dialog util connecting is in progress
    dlg->show();

    // checking result for error
    if (!res.is_ok()) {
        throw std::runtime_error{res.error()};
    }

    return true;
}


bool application::ask_and_disconnect() {
    // asking and closing loaded/launched targets
    if (!ask_and_close()) {
        return false;
    }

    assert(state() == state_t::initial && "invalid state after ask_and_close");

    // disconnecting from platform
    dbg_->disconnect_from_platform();
    return true;
}


bool application::ask_and_load_target(const std::filesystem::path & exe_path,
                                      const launch_options & def_l_opts,
                                      const platform_connection_options & plat_opts) {
    // asking for closing current executable
    if (!ask_and_close()) {
        return false;
    }

    // connecting to platform if specified in options
    if (plat_opts.plat != nullptr) {
        if (!ask_and_connect(plat_opts)) {
            return false;
        }
    }

    // loading target
    load_target(exe_path, def_l_opts);

    return true;
}


void application::load_target(const std::filesystem::path & exe_path,
                              const launch_options & def_lopts) {

    assert(state() == state_t::initial && "Invalid application state");

    // saving default launch options
    def_launch_opts_ = def_lopts;

    // start loading target with handler
    async::result<> res{"unknown error"};
    dbg_->load_target(exe_path, [&res](const auto & r) {
        res = r;
    });


    // displaying loading progress and waiting until loading is complete
    display_progress_while(make_loading_progress(exe_path), state_t::loading);

    assert(state() == state_t::loaded || state() == state_t::initial &&
                                      "Invalid state after loading target");

    // throwing exception if loading failed
    if (!has_target()) {
        if (res.is_ok()) {
            res.set_error("unknown error");
        }

        std::ostringstream msg;
        msg << "can't load executable " << exe_path << " for debugging: " << res.error();
        throw std::runtime_error(msg.str());
    }
}


void application::load_target(const std::filesystem::path & exe_path,
                              const std::filesystem::path & work_dir,
                              const std::vector<std::string> & cmd_args) {
    launch_options lopts;
    lopts.set_work_dir(work_dir);
    std::copy(cmd_args.begin(), cmd_args.end(), std::back_inserter(lopts.launch_args()));
    load_target(exe_path, lopts);
}


void application::do_attach(const std::variant<unsigned long, std::string> & targ) {
    assert((state() == state_t::initial || state() == state_t::loaded) &&
           "Invalid application state");

    // starting attaching, and recoring attaching result on completion

    async::result<> res{"unknown error"};

    dbg_->attach(targ, [&res](auto && r) {
        res = r;
    });

    std::string targ_name;
    if (auto pid = std::get_if<unsigned long>(&targ)) {
        targ_name = std::to_string(*pid);
    } else if (auto nm = std::get_if<std::string>(&targ)) {
        targ_name = *nm;
    } else {
        assert(false && "unknown attach target type");
    }

    // showing attaching dialog while state is loading
    display_progress_while(make_attaching_progress(targ_name), state_t::loading);

    // showing attaching dialog while state is launching
    display_progress_while(make_attaching_progress(targ_name), state_t::launching);

    if (!has_process()) {
        // attaching failed. Throwing error
        if (res.is_ok()) {
            res.set_error("unknown error");
        }
        throw std::runtime_error(res.error());
    }

    // if attaching success and process is stopped then resuming execution
    if (state() == application::state_t::stopped) {
        run();
    }
}


bool application::ask_and_attach(const std::variant<unsigned long, std::string> & targ,
                                 const platform_connection_options & plat_opts) {
    // if process is running then terminating it
    if (!ask_terminate()) {
        return false;
    }

    // connecting to platform if requested
    if (plat_opts.plat != nullptr) {
        if (!ask_and_connect(plat_opts)) {
            return false;
        }
    }

    // attaching
    do_attach(targ);

    return true;
}


void application::close_target() {
    dbg_->close_target([](auto && ...){});
}


bool application::close() {
    assert(has_target() && "application::close can't be called without target");

    // terminating process if running
    if (!ask_terminate()) {
        return false;
    }

    assert(state() == state_t::loaded && "invalid application state after terminating proces");

    // start closing executable
    close_target();

    // show closing dialog until executable is closed
    display_progress_while(make_closing_progress(), state_t::unloading);

    assert(state() == state_t::initial && "invalid application state after application::close");

    return true;
}


bool application::ask_and_close() {
    // display confirmation dialog if executable already opened
    if (state() != cxxdbg::dbg::application::state_t::initial) {
        if (!ask_close_confirmation())
            return false;

        // close opened executable
        if (!close()) {
            // closing executable was canceled
            return false;
        }
    }

    return true;
}


unsigned long application::launch(const launch_options & lopts, bool save_default) {

    assert(state() == state_t::loaded &&
           "application::launch can be called only in loaded state");

    // saving default launch options
    if (save_default) {
        def_launch_opts_ = lopts;
    }

    // displaying launching progress and waiting unitl launch is complete
    auto dlg = make_launching_progress();
    auto pdlg = dlg.get();

    // launching
    async::result<unsigned long> res{"unknown error"};
    dbg_->launch(lopts, [pdlg, &res](auto && r) {
        res = r;
        pdlg->close();
    });

    assert(state() == application::state_t::launching &&
           "Invalid application state after starting launching");

    // displaying modal dialog. It will be closed in completion handler
    // passed to debugger::launch method
    dlg->show();

    assert((state() == application::state_t::loaded ||
            state() == application::state_t::running) &&
           "Invalid application state after starting launching");

    if (!res.is_ok()) {
        // launch failed
        std::ostringstream msg;
        msg << "Can't launch application: " << res.error();
        throw std::runtime_error{msg.str()};
    }

    return res.value();
}


void application::run() {
    dbg_->run();
}


void application::stop() {
    dbg_->stop();
}


void application::do_terminate() {
    dbg_->terminate();
}


void application::terminate() {
    assert(has_process() &&
           "application::terminate can be called only if has_process() == true");

    if (state() == state_t::running) {
        // start stopping process if running
        stop();

        // displaying modal stopping dialog until process is stopped
        display_progress_while(make_stopping_progress(), state_t::running);
    }

    // checking if process is already terminated (it may exit after call to stop())
    if (state() == state_t::loaded) {
        return;
    }

    // starting terminating process
    do_terminate();

    // displaying modal terminating dialog until process is terminated
    display_progress_while(make_terminating_progress(false), state_t::terminating);

    assert(state() == state_t::loaded && "Invalid application state after terminating");
}


bool application::ask_terminate() {
    // doing nothing if executable is not running
    if (state() == application::state_t::initial ||
        state() == application::state_t::loaded) {
        return true;
    }

    // show confirmation message
    if (!ask_terminate_confirmation())
        return false;

    // terminating
    terminate();
    return true;
}


void application::do_detach() {
    dbg_->detach();
}


void application::detach() {
    assert(has_process() &&
           "application::detach can be called only if has_process() == true");

    if (state() == state_t::running) {
        // start stopping process if running
        stop();

        // displaying modal stopping dialog until process is stopped
        display_progress_while(make_stopping_progress(), state_t::running);
    }

    // detaching from process
    do_detach();

    // displaying model detaching dialog until process is detached
    display_progress_while(make_detaching_progress(), state_t::detaching);

    assert(state() == state_t::loaded && "Invalid application state after detaching");
}


void application::step_into(boost::logic::tribool avoid_nodebug,
                            const std::string & target,
                            const std::string & avoid_regex,
                            const std::string & step_through_regex) {

    dbg_->step_into(avoid_nodebug, target, avoid_regex, step_through_regex);
}


void application::step_into_call_target(const std::string & target_name) {
    dbg_->step_into_call_target(target_name);
}


void application::step_over(const std::string & step_through_regex) {
    dbg_->step_over(step_through_regex);
}


void application::step_out(boost::tribool avoid_nodebug,
                           const std::string & step_through_regex) {
    dbg_->step_out(avoid_nodebug, step_through_regex);
}


void application::step_inst_into() {
    dbg_->step_inst_into();
}


void application::step_inst_over() {
    dbg_->step_inst_over();
}


void application::do_step_until(unsigned int line) {
    dbg_->step_until(line);
}


void application::step_until() {
    // asking line number and checking that user clicked ok
    unsigned int line_num = 0;
    if (!ask_step_until_line_number(line_num))
        return;

    // performing step until on current target
    do_step_until(line_num);
}


boost::signals2::connection application::connect_on_event(
        const std::function<void(const std::string &)> & handler) {
    return dbg_->connect_on_event([handler](const std::string & evnt) {
        handler(evnt);
    });
}


bool application::has_target() const {
    return dbg_->has_target();
}


target & application::current_target() {
    return dbg_->current_target();
}


const target & application::current_target() const {
    return dbg_->current_target();
}


std::string application::exe_path() {
    return current_target().exe_path();
}


launch_options application::def_launch_opts() const {
    return def_launch_opts_;
}


const composite_source_tree_model & application::trees_model() const {
    return *src_tree_mdl_;
}


source_tree & application::trees() {
    return *src_tree_;
}


const source_tree & application::trees() const {
    return *src_tree_;
}


source_model & application::sources() {
    return sources_;
}


const source_model & application::sources() const {
    return sources_;
}


signal_connection application::connect_sources_changed(const sources_changed_handler & handl) {
    return sources_changed_signal_.connect(handl);
}


bool application::can_show_pos(const source_position & pos) {
    // can't show invalid source position
    if (!pos)
        return false;

    // can't show file which does not exist
    if (!fs::exists(pos.file()->path()))
        return false;

    return true;
}


void application::show_pos(const source_position & pos) {
    show_pos(pos.file()->path(), static_cast<size_t>(pos.line() - 1));
}


void application::show_pos(const std::filesystem::path & path, size_t line) {
    documents().show_pos<true>(path, line);
}


const thread_list & application::threads() const {
    return dbg_->threads();
}


const thread * application::current_thread() const {
    return dbg_->current_thread();
}


void application::set_current_thread(const thread * thrd) {
    dbg_->set_current_thread(thrd);
}


const stack_frame * application::current_frame() const {
    return dbg_->current_frame();
}


void application::set_current_frame(const stack_frame * frame) {
    dbg_->set_current_frame(frame);
}


void application::fetch_all_stack_frames() {
    dbg_->fetch_all_stack_frames();
}


signal_connection application::connect_state_changed(const std::function<void()> & handler) {
    return state_changed_signal_.connect(handler);
}


signal_connection application::connect_current_thread_changed(
        const current_thread_changed_handler & handl) {
    return dbg_->current_thread_changed().connect(handl);
}


signal_connection application::connect_current_stack_frame_changed(
        const current_stack_frame_changed_handler & handl) {
    return dbg_->current_frame_changed().connect(handl);
}


breakpoint_list & application::breakpoints() {
    return dbg_->breakpoints();
}


const breakpoint_list & application::breakpoints() const {
    return dbg_->breakpoints();
}


watch_list & application::locals() {
    return dbg_->locals();
}


custom_watch_list & application::watch() {
    return  dbg_->watch();
}


const term_settings application::term_sett() const {
    return dbg_->term_sett();
}


void application::set_term_sett(const term_settings & sett) {
    dbg_->set_term_sett(sett);
}


const debug_settings & application::debug_sett() const {
    return dbg_->debug_sett();
}


void application::set_debug_setttings(const debug_settings & sett) {
    dbg_->set_debug_sett(sett);
}


cli::app_command_interpreter & application::interp() {
    return *interp_;
}


debugger_terminal & application::dbg_term() {
    return term_;
}


void application::notify_module_load_start(const std::filesystem::path & mod) {
    std::ostringstream msg;
    msg << "Loading module '" << mod << "'";
    show_status_message(msg.str());
}


void application::notify_module_load_finish() {
    clear_status_message();
}


std::wstring application::status_text() const {
    return dbg_->status_text();
}


dbg_terminal & application::exe_term() {
    return dbg_->exe_term();
}


void application::display_progress_while(const std::shared_ptr<progress_dialog> & dlg, state_t st) {
    if (state() != st)
        return;

    progress_dialog * pdlg = dlg.get();
    scoped_signal_connection con{connect_state_changed([this, pdlg, st] {
        if (state() != st)
            pdlg->close();
    })};

    dlg->show();
}


void application::on_target_changed(const async::result<target_sp> & res,
                                    const std::vector<module_info> & mods,
                                    const source_position & main_pos) {

    if (!has_target()) {
        // target unloaded
        return;
    }

    // merge sources from loaded target modules
    on_modules_loaded(mods);

    // opening main code viewer
    if (main_pos) {
        show_pos(main_pos);
    }

    // emitting sources changes signal
    sources_changed_signal_();

    // listening for new modules in target
    current_target().modules_loaded().connect([this](const auto & mods) {
        this->on_modules_loaded(mods);
    });
}


void application::on_modules_loaded(const std::vector<module_info> & mods) {

    // updating sources

    for (auto & mod: mods) {
        source_tree_builder builder;
        std::size_t count = 0;
        for (auto & sf : mod.sources()) {
            // ignore <built-in> item
            if (sf.filename() == L"<built-in>") {
                continue;
            }

            // put sources with relative path in special root item
            fs::path tree_path = sf;
            if (!sf.is_absolute()) {
                tree_path = "/<includepath>/" / sf;
            }

            source_file * file = sources_.source(tree_path);
            builder.add(file);

            ++count;
        }

        src_tree_->merge(builder.tree());
    }

    src_tree_->update_sorted_sources(sources_.sorted_sources());

    sources_changed_signal_();
}


}
