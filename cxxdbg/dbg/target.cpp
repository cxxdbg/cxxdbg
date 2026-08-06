// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file target.cpp
/// Contains implementation of target class.

#include "target.hpp"
#include "address_breakpoint.hpp"
#include "application.hpp"
#include "breakpoint.hpp"
#include "breakpoint_list.hpp"
#include "exception_thrown_breakpoint.hpp"
#include "exception_caught_breakpoint.hpp"
#include "breakpoint_info.hpp"
#include "breakpoint_location.hpp"
#include "compile_unit_info.hpp"
#include "debugger.hpp"
#include "function_breakpoint.hpp"
#include "module.hpp"
#include "module_info.hpp"
#include "process_context.hpp"
#include "source_position.hpp"
#include "source_position_breakpoint.hpp"
#include "stack_frame.hpp"
#include "stop_reason.hpp"
#include "target_impl.hpp"
#include "thread.hpp"
#include "thread_list.hpp"
#include "watchpoint.hpp"
#include "cxxdbg/util/convert.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/asio/io_context.hpp>
#include "cxxdbg/boost_process/process.hpp"
#include <boost/dll.hpp>
#include <filesystem>
#include <sstream>


namespace bp = boost::process;
namespace fs = std::filesystem;


namespace cxxdbg::dbg {


target::target(debugger & d,
               async::event_queue & main_thrd_queue,
               const std::shared_ptr<target_impl> & tgt,
               source_model & src_model,
               breakpoint_list & bplist,
               const std::vector<module_info> & mods,
               const std::filesystem::path & epath):
dbg_(d),
main_thread_queue_(main_thrd_queue),
targ_(tgt),
src_model_(src_model),
threads_(std::make_unique<thread_list>(src_model_)),
bplist_(bplist),
exe_path_(epath),
pid_{0},
stop_rsn_{new stop_reason},
exit_code_{0} {

    // trying get exe path from modules if not specified
    if (exe_path_.empty() && !mods.empty()) {
        exe_path_ = mods.front().mod_path();
    }

    // starting listening for implementation state changed signal
    impl_state_changed_con_ =
    targ_->connect_state_changed([this](target_impl::state_t st,
                                        const process_context * ctx,
                                        int exit_code) {
        on_impl_state_changed(st, ctx, exit_code);
    });


    // listening for modules changed signal in implementation, and updating
    // sources when modules changed

    impl_modules_loaded_con_ = targ_->modules_loaded().connect(
    [this](const std::vector<module_info> & mods) {
        add_modules(mods);
    });

    impl_modules_unloaded_con_ = targ_->modules_unloaded().connect(
    [this](const auto & mods) {
        // TODO
    });


    // notifying users about starting module loading
    impl_start_module_load_con_ = targ_->module_load_started().connect([this](const auto & mod) {
        dbg_.notify_module_load_start(mod);
    });

    // notifying users about module loading finish
    impl_finish_module_load_con_ = targ_->module_load_finished().connect([this] {
        dbg_.notify_module_load_finish();
    });


    // listening for stdout
    targ_->stdout_received().connect([this](const std::string & data) {
        stdout_received()(data);
    });

    // listening for stderr
    targ_->stderr_received().connect([this](const std::string & data) {
        stderr_received()(data);
    });


    // updating list of sources
    add_modules(mods);
}


target::~target() {
    // target can be destroyed only in invalid state
    assert(state() == state_t::invalid && "Can't destroy target in not invalid state");
}


std::string target::exe_path() {
    return exe_path_.string();
}


std::vector<const char*> target::find_funcs(const std::string & prefix) const {
    std::vector<const char*> res;

    if (prefix.size() < 3)
        return res;

    for (const auto & mod : mods_) {
        mod->find_funcs(prefix, res);
    }

    return res;
}


target::connection target::connect_sources_changed(const sources_changed_handler & handl) {
    return sources_changed_signal_.connect(handl);
}


const std::vector<std::string> & target::get_call_targets() const {
    return call_targets_;
}


target::state_t target::state() const {
    switch(targ_->state()) {
    case target_impl::state_t::invalid:
        return state_t::invalid;
    case target_impl::state_t::loaded:
        // if we are launching exec_stub executable then state for user is launching
        if (launch_exec_stub_thrd_)
            return state_t::launching;
        else
            return state_t::loaded;
    case target_impl::state_t::launching:
        return state_t::launching;
    case target_impl::state_t::running:
        return state_t::running;
    case target_impl::state_t::stopped:
        return state_t::stopped;
    case target_impl::state_t::terminating:
        return state_t::terminating;
    case target_impl::state_t::detaching:
        return state_t::detaching;
    case target_impl::state_t::unloading:
        return state_t::unloading;
    default:
        assert(false && "Unknown implementation state");
        return state_t::invalid;
    }
}


const stop_reason & target::stop_rsn() const {
    assert(state() == state_t::stopped &&
           "stop_reason can be called only in stopped state");
    assert(stop_rsn_.get() != nullptr && "null stop reason");
    return *stop_rsn_;
}


int target::exit_code() const {
    return exit_code_;
}


void target::launch(const launch_options & lopts,
                    const async::result_handler<unsigned long> & handler) {

    launched_in_term_ = false;

    // launching implementation
    targ_->launch(lopts, handler);
}


void target::launch_terminal(const launch_options & lopts,
                             const term_settings & term_sett,
                             const async::result_handler<unsigned long> & handl) {

    // creating exec_stub server for exchanging data with exec_stub executable and debuggee
    // we need explicitly remove old server if it exists because we always use same name
    // for shared memory region
    exec_stub_srv_.reset();
    exec_stub_srv_ = std::make_unique<exec_stub::exec_stub_server>();

    // getting path to exec_stub executable

    std::filesystem::path exec_stub_path{boost::dll::program_location().string()};
    exec_stub_path.remove_filename();
    exec_stub_path /= "cxxdbg-exec-stub";

    // building list of arguments for terminal

    fs::path term_path;
    std::vector<std::string> term_args;

    if (term_sett.tkind() == term_settings::term_kind::xterm) {
        term_path = term_sett.xterm_path();

        if (!term_sett.close_xterm()) {
            term_args.push_back("-hold");
        }

        // setting title of terminal window to executable name
        term_args.push_back("-title");
        term_args.push_back(exe_path_.filename().string());

        term_args.push_back("-e");
    } else {
        std::vector<std::string> custom_term_args;
        boost::algorithm::split(custom_term_args,
                                term_sett.custom_term_cmd(),
                                boost::is_any_of("\t "),
                                boost::token_compress_on);
        if (!custom_term_args.empty()) {
            term_path = custom_term_args.front();
            std::copy(custom_term_args.begin() + 1,
                      custom_term_args.end(),
                      std::back_inserter(term_args));
        }
    }

    // exec_stub args
    term_args.push_back(exec_stub_path.string());
    term_args.push_back(std::string("--shm=") + exec_stub_srv_->shared_mem_name());
    term_args.push_back("--args");
    term_args.push_back(exe_path_.string());
    std::copy(lopts.launch_args().begin(), lopts.launch_args().end(), std::back_inserter(term_args));

    // creating stream for reading stdout/stderr
    auto child_out = std::make_shared<bp::ipstream>();

    // launching exec_stub in xterm
    std::shared_ptr<bp::child> child;

    try {
        child = std::make_shared<bp::child>(bp::exe = term_path.wstring(),
                                            bp::args = term_args,
                                            bp::start_dir = lopts.work_dir().string(),
                                            (bp::std_out & bp::std_err) > *child_out);
    }
    catch(std::exception & err) {
        throw std::runtime_error(std::string("Can't launch terminal: ") + err.what());
    }

    // checking that process is running
    if (!child->running()) {
        std::ostringstream msg;
        msg << "Can't launch terminal using command";
        for (auto it = term_args.begin(), end = term_args.end(); it != end; ++it) {
            msg << " '" << *it << "'";
        }
        throw std::runtime_error(msg.str());
    }

    // waiting for exec_stub executable pid
    assert(!launch_exec_stub_thrd_ && "Launch cxxdbg-exec-stub thread alredy exists");
    launch_exec_stub_thrd_.reset(new std::thread([this, child, child_out, handl] {
        // waiting for PID and checking process status

        unsigned long pid = 0;
        unsigned long tid = 0;

        for (unsigned int count = 0; count < 20; ++count) {
            // waiting for PID from exec stub server
            std::chrono::nanoseconds dur =
                    std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(500));
            std::tie(pid, tid) = exec_stub_srv_->wait_pid(dur);
            if (pid != 0) {
                break;
            }

            // checking exit status
            if (!child->running()) {
                pid = static_cast<unsigned long>(-1);
                break;
            }
        }

        // detaching from child before destroying handle poiting to it
        child->detach();

        // notifying main thread
        main_thread_queue_.post([this, pid, handl, child, child_out] {
            on_exec_stub_launched(*child_out, pid, [this, pid, handl](const auto & res) {
                // We successfully attached to process here. Sending continue signal to
                // child process (created by fork in exec_stub) via the exec_stub server structure
                // We can't destroy exec stub server here because child process still waits on
                // semaphore. It will be destroyed together with target.
                if (res.is_ok()) {
                    exec_stub_srv_->notify_continue();
                }

                handl(res.to(pid));
            });
        });
    }));


    // sending notification about state changed
    // launch_exec_stub_thrd != null -> state_t::launching
    state_changed_signal_();
}


void target::attach(const std::variant<unsigned long, std::string> & targ,
                    const async::result_handler<> & h) {

    launched_in_term_ = true;

    // performing attach in implementation
    targ_->attach(targ, h);
}


void target::run() {
    // running implementation
    targ_->run();
}


void target::stop() {
    targ_->stop();
}


void target::terminate() {
    // removing thread info
    threads().clear();

    // terminating implementation
    targ_->terminate();
}


void target::detach() {
    // removing thread info
    threads().clear();

    // detaching implementation
    targ_->detach();
}


void target::close(const std::function<void()> & handl) {
    targ_->close(handl);
}


void target::step_into(bool avoid_nodebug,
                       const std::string & target_func,
                       const std::string & avoid_regex,
                       const std::string & step_through_regex) {

    // getting current thread ID
    unsigned long thread_id = threads().current_thread()->id();

    // stepping into in implementation
    targ_->step_into(thread_id,
                     avoid_nodebug,
                     target_func,
                     avoid_regex,
                     step_through_regex);
}


void target::step_over(const std::string & step_through_regex) {
    // getting current thread ID
    unsigned long thread_id = threads().current_thread()->id();

    // stepping over in implementation
    targ_->step_over(thread_id, step_through_regex);
}


void target::step_out(bool avoid_nodebug, const std::string & step_through_regex) {
    // getting current thread ID
    unsigned long thread_id = threads().current_thread()->id();

    // stepping out in implementation
    targ_->step_out(thread_id, avoid_nodebug, step_through_regex);
}


void target::step_inst_into() {
    // getting current thread ID
    unsigned long thread_id = threads().current_thread()->id();

    // stepping in implementation
    targ_->step_inst_into(thread_id);
}


void target::step_inst_over() {
    // getting current thread ID
    unsigned long thread_id = threads().current_thread()->id();

    // stepping in implementation
    targ_->step_inst_over(thread_id);
}


void target::step_until(unsigned int line) {
    // getting current thread ID
    unsigned long thread_id = threads().current_thread()->id();

    // stepping in implementation
    targ_->step_until(thread_id, line);
}


const thread_list & target::threads() const {
    return *threads_;
}


thread_list & target::threads() {
    return *threads_;
}


const thread * target::current_thread() const {
    return threads().current_thread();
}


void target::set_current_thread(const thread * thrd) {
    assert(state() == state_t::stopped && "set_current_thread can be called only in stopped state");

    // looking for thread index
    std::size_t index = 0;
    for (auto it = threads().begin(), end = threads().end(); it != end; ++it) {
        if (*it == thrd)
            break;
        ++index;
    }

    // setting current thread in list of threads
    threads().set_current_thread(thrd);

    // setting current thread in target implementation
    targ_->set_current_thread(index);

    current_stack_frame_changed_signal_();
}


const stack_frame * target::current_frame() const {
    if (state() != state_t::stopped)
        return nullptr;

    auto thrd = threads().current_thread();
    assert(thrd != nullptr && "current thread is null");

    return thrd->current_frame();
}


void target::set_current_frame(const stack_frame * frame) {
    assert(state() == state_t::stopped && "set_current_frame can be called only in stopped state");

    // looking for frame index
    std::size_t frame_index = 0;
    thread * thrd = threads().current_thread();
    assert(thrd != nullptr && "Null thread");
    for (auto && f : thrd->call_stack()) {
        if (f == frame)
            break;
        ++frame_index;
    }

    // setting current frame in thread
    thrd->set_current_frame(frame);

    // setting current frame in target implementation
    targ_->set_current_stack_frame(frame_index);

    // emitting current stack frame changed signal
    current_stack_frame_changed_signal_();
}


void target::fetch_all_stack_frames() {
    assert(state() == state_t::stopped &&
           "fetch_all_stack_frames can be called only in stopped state");

    targ_->fetch_all_frames([this](const auto & tinfo) {
        threads().update_current(*tinfo);
    });
}


watch_list_impl & target::locals() {
    return targ_->locals_list();
}


custom_watch_list_impl & target::watch() {
    return targ_->watch_list();
}


watch_list_impl & target::registers() {
    return targ_->registers_list();
}


void target::send_stdin(const std::string & data) {
    targ_->send_stdin(data);
}


void target::read_memory(uint64_t addr, size_t size, const async::result_handler<std::vector<char>> & h) {
    targ_->read_memory(addr, size, h);
    
}


source_position target::convert_source_position_info(source_model & src_mdl,
                                                     const source_position_info & pos_info) {
    if (!pos_info) {
        // source position is invalid
        return source_position();
    }

    // resolving source file
    return source_position(src_mdl.source(pos_info.path()), pos_info.line());
}


code_position target::convert_code_position_info(source_model & src_mdl,
                                                 const code_position_info & pos_info) {
    return code_position(pos_info.load_addr(),
                         pos_info.file_addr(),
                         pos_info.func_name(),
                         convert_source_position_info(src_mdl, pos_info.src_pos()));
}


void target::clear_threads() {
    threads().clear();
}


boost::signals2::connection target::connect_state_changed(const state_changed_handler &handl) {
    return state_changed_signal_.connect(handl);
}


boost::signals2::connection target::connect_current_thread_changed(
        const current_thread_changed_handler & handl) {
    return threads().current_thread_changed().connect(handl);
}


target::connection target::connect_current_stack_frame_changed(
        const current_stack_frame_changed_handler & handl) {
    return current_stack_frame_changed_signal_.connect(handl);
}


unsigned long target::pid() {
    return pid_;
}


void target::on_impl_state_changed(target_impl::state_t st,
                                   const process_context * ctx,
                                   int ecode) {
 
    if (ctx != nullptr) {
        // updating stop reason
        std::shared_ptr<stop_reason> new_stop_reason;

        switch (ctx->stop_reason()) {
        case process_context::stop_reason_unknown:
            new_stop_reason.reset(new stop_reason);
            break;
        case process_context::stop_reason_trace:
            new_stop_reason.reset(new stop_reason_trace);
            break;
        case process_context::stop_reason_breakpoint:
            new_stop_reason.reset(new stop_reason_breakpoint);
            break;
        case process_context::stop_reason_watchpoint:
            new_stop_reason.reset(new stop_reason_watchpoint{ctx->watchpoint_num()});
                //app_.
            break;
        case process_context::stop_reason_signal:
            new_stop_reason.reset(new stop_reason_signal{ctx->stop_signal()});
            break;
        case process_context::stop_reason_exec:
            new_stop_reason.reset(new stop_reason_exec);
            break;
        case process_context::stop_reason_crashed:
            new_stop_reason.reset(new stop_reason_crash);
            break;
        default:
            assert(false && "unknown stop reason");
            break;
        }

        // setting process id
        pid_ = ctx->pid();

        stop_rsn_ = new_stop_reason;

        // updating threads
        threads().update(ctx->threads());

        // updating breakpoints
        bplist_.update_breakpoints(ctx->breakpoints(),
                                   stop_rsn_->is_wpoint(),
                                   current_frame());

        // update call targets
        const auto & thread_list_info = ctx->threads();
        const auto & thread_info = thread_list_info[thread_list_info.current_thread()];

        const std::vector<std::string> & new_targets = thread_info.call_targets();

        if (call_targets_ != new_targets) {
            call_targets_ = new_targets;
            //call_targets_changed_(call_targets_);
        }


        // updating sources if new module info received
        if (ctx->modules()) {
            add_modules(*ctx->modules());
        }
    }

    // setting exit code
    if (state() == state_t::loaded) {
        exit_code_ = ecode;
    }

    // removing threads if process exited
    if (state() != state_t::running &&
        state() != state_t::stopped) {
        threads().clear();
    }

    // emitting state changed and related signals
    state_changed_signal_();
    current_thread_changed_signal_();
    current_stack_frame_changed_signal_();
}


void target::on_exec_stub_launched(std::istream & child_stderr,
                                   unsigned long pid,
                                   const async::result_handler<> & handl) {

    assert(launch_exec_stub_thrd_ && "Launching cxxdbg-exec-stub thread should exist");

    // removing launch thread
    launch_exec_stub_thrd_->join();
    launch_exec_stub_thrd_.reset();

    if (pid == 0 || pid == -1) {
        // launch failed, notifying users about chaning state
        // (launch_exec_stub_thrd_ == null -> state_t::loaded)
        // and throwing error
        state_changed_signal_();

        std::string error_msg;

        if (pid == 0) {
            // wait timed out

            error_msg = "Waiting for exec_stub executable timed out";
        } else {
            // cxxdbg-exec-stub or terminal exited. Trying read child stderr
            std::ostringstream msg;
            msg << "can't start cxxdbg-exec-stub in terminal. Child process output:\n\n";
            while (true) {
                char c = static_cast<char>(child_stderr.get());
                if (!child_stderr)
                    break;

                msg.put(c);
            }

            error_msg = msg.str();
        }

        handl(async::result<>{error_msg});
        return;
    }

    // starting attaching to process with received PID
    attach(pid, handl);
}


void target::add_modules(const std::vector<module_info> & mods) {
    // adding modules
    for (const auto & minf : mods) {
        mods_.push_back(std::make_shared<module>(minf));
    }

    // notifying observers about new modules
    modules_loaded()(mods);
}


}
