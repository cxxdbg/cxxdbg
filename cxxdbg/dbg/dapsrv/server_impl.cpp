// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file server_impl.cpp
/// Contains implementation of the server_impl class.

#include "server_impl.hpp"
#include "client_term_debug_child_process.hpp"
#include "client_term_exec_stub_launcher.hpp"
#include "debug_child_process.hpp"
#include "nodebug_child_process.hpp"
#include "log.hpp"
#include "cxxdbg/dbg/core/regex_lists_defaults.hpp"
#include "cxxdbg/exec_stub/exec_stub_server.hpp"
#include "cxxdbg/proc/async_launcher.hpp"
#include "cxxdbg/proc/launch_parameters.hpp"
#include "cxxdbg/util/convert.hpp"
#include "cxxdbg/util/print.hpp"
#include "cxxdbg/boost_process/process.hpp"
#include <boost/asio/post.hpp>
#include <boost/dll.hpp>
#include <memory>
#include <sstream>


namespace fs = std::filesystem;


namespace cxxdbg::dbg::dapsrv {


server_impl::~server_impl() = default; 


void server_impl::handle_input_closed() {
    CXXDBG_DAP_LOG_INFO << "input closed";
    if (!disconnect_requested_) {
        disconnect(false, false);
    }
}


dap::initialize_response server_impl::initialize(const dap::initialize_request & req) {
    // post initialized event to queue of events
    boost::asio::post(exec_, [this] {
        send_initialized();
    });

    return {
        {
            dap::capability::configuration_done_request,
            dap::capability::function_breakpoints,
            dap::capability::conditional_breakpoints,
            dap::capability::hit_conditional_breakpoints,
            dap::capability::step_in_targets_request,
            dap::capability::modules_request,
//            dap::capability::restart_request,
            dap::capability::exception_options,
            dap::capability::value_formatting_options,
            dap::capability::terminate_debuggee,
//            dap::capability::delayed_stack_trace_loading,
            dap::capability::loaded_sources_request,
            dap::capability::terminate_request,
//            dap::capability::data_breakpoints,        // data breakpoints not supported in vscode yet
            dap::capability::read_memory_request,
            dap::capability::disassemble_request,
            dap::capability::instruction_breakpoints
        },
        {
            {L"thrown", L"Exception thrown", false},
            {L"caught", L"Exception caught", false}
        },
    };
}


void server_impl::disconnect(bool restart, bool terminate) {
    // first of all, we need disable sending log to output because it
    // will cause crashes in random places after closing connection
    log_con_.disconnect();

    CXXDBG_DAP_LOG_INFO << "disconnect [restart = "<< restart << ", terminate = " << terminate << "]";

    // if there is no child process then just notify server that we want exit
    if (!child_) {
        notify_exited();
        return;
    }

    // setting disconnect flag
    disconnect_requested_ = true;
}


void server_impl::launch(bool no_debug,
                         const dap::json_t & parameters,
                         const std::function<void (const std::string &)> & comp_handler) {
    if (child_) {
        throw std::runtime_error("target already exists");
    }

    if (parameters.count("program") == 0) {
        throw std::runtime_error("required parameter 'program' is not set in launch configuration");
    }

    // handling log to output parameter
    if (parameters.value("logToOutput", true)) {
        log_con_ = log_.record_received().connect([this](const std::string & msg) {
            // Can be called in any thread. We have to post it to main work thread
            // because we can send output only from there.
            boost::asio::post(exec_, [this, msg] {
                send_output("console", msg + "\n");
            });
        });
    }

    // parsing environment variables
    std::map<std::wstring, std::wstring> env;
    if (parameters.count("environment") != 0) {
        auto & jenv = parameters.at("environment");
        for (auto && var : jenv) {
            env.emplace(var.at("name").get<std::wstring>(), var.value("value", std::wstring()));
        }
    }

    if (no_debug) {
        auto platform = parameters.value("platform", std::string{});

        auto program = parameters.at("program").get<std::wstring>();

        // setting up launch parameters
        proc::launch_parameters launch_pars;
        launch_pars.exe = program;
        launch_pars.args = parameters.value("args", std::vector<std::wstring>{});
        launch_pars.work_dir = parameters.value("cwd", std::wstring{});
        launch_pars.env = env;

        // creating process launcher for platform and adjusting launch parameters if needed
        std::unique_ptr<proc::async_launcher> launcher;
        if (platform.empty() || platform == "host") {
            launcher = std::make_unique<client_term_exec_stub_launcher>(*this);
        } else {
            std::ostringstream msg;
            msg << "launching without debugging is not implemented for platform '" << platform << "'";
            throw std::runtime_error{msg.str()};
        }

        // creating nodebug child process
        child_ = std::make_unique<nodebug_child_process>(std::move(launcher), std::move(launch_pars));

        prepare_child();

    } else {
        // checking if we should launch in terminal
        bool launch_in_term = parameters.value("launchInTerminal", true);

        // disable launching in terminal if platform is specified and is not equal to host
        if (parameters.value("platform", "host") != "host") {
            launch_in_term = false;
        }

        // creating child for debugging
        debug_child_process * dchp = nullptr;

        if (launch_in_term) {
            auto proc = std::make_unique<client_term_debug_child_process>(exec_, std::make_unique<client_term_process_launcher>(*this));
            dchp = proc.get();
            child_ = std::move(proc);
        } else {
            auto proc = std::make_unique<debug_child_process>(exec_);
            dchp = proc.get();
            child_ = std::move(proc);
        }

        prepare_child();

        // preparing child for launching
        dchp->prepare_launch(parameters.at("program").get<std::wstring>(),
                             parameters.value("cwd", std::wstring{}),
                             parameters.value("args", std::vector<std::wstring>{}),
                             env,
                             parameters,
                             source_breakpoints_,
                             function_breakpoints_,
                             instruction_breakpoints_,
                             exception_breakpoint_filters_);
    }

    comp_handler({});
}


void server_impl::attach(const dap::json_t & parameters) {
    auto pid  = parameters.value("pid", size_t{0});
    auto program = parameters.value("program", std::wstring{});

    CXXDBG_DAP_LOG_INFO << "attach: pid = " << pid << ", program = '" << program << "'";

    // creating child for debugging
    debug_child_process * dchp = nullptr;
    {
        auto proc = std::make_unique<debug_child_process>(exec_);
        dchp = proc.get();
        child_ = std::move(proc);
    }

    prepare_child();

    // attaching to process
    dchp->attach(pid,
                 program,
                 parameters,
                 source_breakpoints_,
                 function_breakpoints_,
                 instruction_breakpoints_,
                 exception_breakpoint_filters_);
}


bool server_impl::process_continue(size_t thread_id) {
    return child().process_continue(thread_id);
}


void server_impl::configuration_done(const std::function<void (const std::string &)> & comp_handler) {
    CXXDBG_DAP_LOG_INFO << "configuration done";
    child().process_configuration_done(comp_handler);
}


std::vector<dap::completion_item>
server_impl::completions(size_t frame_id, const std::wstring & text, size_t column, size_t line) {
    return {};
}


dap::data_breakpoint_info server_impl::get_data_breakpoint_info(size_t var, const std::wstring & name) {
    return {};
}


dap::evaluate_response server_impl::evaluate(const std::wstring & expr,
                                             size_t frame_id,
                                             const std::wstring & context,
                                             const dap::value_format & fmt) {
    CXXDBG_DAP_LOG_INFO << "evaluate '" << expr << "'";
    return child().evaluate(expr, frame_id, context, fmt);
}


void server_impl::process_goto(size_t thread_id, size_t target_id) {
    throw std::runtime_error{"goto request is not supported"};
}


std::vector<dap::goto_target> server_impl::goto_targets(const path_t & src, size_t line, size_t column) {
    return {};
}


std::vector<dap::path_t> server_impl::loaded_sources() {
    return {};
}


size_t server_impl::modules(size_t start, size_t cnt, std::vector<dap::module> & res) {
    return 0;
}


void server_impl::next(size_t thread_id, dap::stepping_granularity granularity) {
    CXXDBG_DAP_LOG_INFO << "next " << thread_id;
    if (granularity == dap::stepping_granularity::instruction) {
        child().next_inst(thread_id);
    } else {
        child().next(thread_id);
    }
}


void server_impl::pause(size_t thread_id) {
    CXXDBG_DAP_LOG_INFO << "pause " << thread_id;
    child().pause(thread_id);
}


dap::read_memory_result server_impl::read_memory(std::wstring_view mem_ref, int64_t offset, uint64_t cnt) {
    return child().read_memory(mem_ref, offset, cnt);
}


std::vector<dap::scope> server_impl::scopes(size_t frame_id) {
    return child().scopes(frame_id);
}


// /// Processes source request
// std::wstring server_impl::source(size_t src_id, std::wstring & mime_type) {
//     return {};
// }


std::vector<dap::breakpoint> server_impl::set_breakpoints(const path_t & src,
                                                          const std::vector<dap::source_breakpoint> & bpts,
                                                          const std::vector<size_t> & line_numbers,
                                                          bool source_modified) {
    if (child_) {
        child().set_breakpoints(src, bpts, line_numbers, source_modified);

        std::vector<dap::breakpoint> res;
        for (auto && bp : bpts) {
            auto id = next_dap_bp_id_++;
            res.push_back({id, true, {}, {}, bp.line});
        }

        return res;
    }

    // if there is no current target then just saving breakpoint info and returning result
    std::vector<dap::breakpoint> res;
    std::vector<std::tuple<size_t, dap::source_breakpoint>> save_bps;

    for (auto && bp : bpts) {
        auto id = next_dap_bp_id_++;
        save_bps.push_back({id, bp});
        res.push_back({id, true, {}, {}, bp.line});
    }

    source_breakpoints_[src] = save_bps;

    return res;
}


std::vector<dap::breakpoint> server_impl::set_data_breakpoints(const std::vector<dap::data_breakpoint> & bpts) {
    return {};
}


void server_impl::set_exception_breakpoints(const std::vector<std::wstring> & filters,
                                            const std::vector<dap::exception_options> & opts) {
    if (child_) {
        child().set_exception_breakpoints(filters, opts);
        return;
    }

    exception_breakpoint_filters_ = filters;
}


std::vector<dap::breakpoint> server_impl::set_function_breakpoints(const std::vector<dap::function_breakpoint> & bpts) {
    if (child_) {
        child().set_function_breakpoints(bpts);

        std::vector<dap::breakpoint> res;
        for (auto && bp : bpts) {
            auto id = next_dap_bp_id_++;
            res.push_back({id, true});
        }

        return res;
    }

    // if there is no current target then just saving breakpoint info and returning result
    std::vector<dap::breakpoint> res;
    function_breakpoints_.clear();

    for (auto && bp : bpts) {
        auto id = next_dap_bp_id_++;
        function_breakpoints_.push_back({id, bp});
        res.push_back({id, true});
    }

    return res;
}


std::vector<dap::breakpoint> server_impl::set_instruction_breakpoints(const std::vector<dap::instruction_breakpoint> & bpts) {
    if (child_) {
        child().set_instruction_breakpoints(bpts);

        std::vector<dap::breakpoint> res;
        for (auto && bp : bpts) {
            auto id = next_dap_bp_id_++;
            res.push_back({id, true});
        }

        return res;
    }

    // if there is no current target then just saving breakpoint info and returning result
    std::vector<dap::breakpoint> res;
    instruction_breakpoints_.clear();

    for (auto && bp : bpts) {
        auto id = next_dap_bp_id_++;
        instruction_breakpoints_.push_back({id, bp});
        res.push_back({id, true});
    }

    return res;
}


size_t server_impl::stack_trace(size_t thread_id,
                                size_t first,
                                size_t max_cnt,
                                const dap::stack_frame_format & fmt,
                                std::vector<dap::stack_frame> & res) {
    return child().stack_trace(thread_id, first, max_cnt, fmt, res);
}


void server_impl::step_in(size_t thread_id, size_t target_id, dap::stepping_granularity granularity) {
    CXXDBG_DAP_LOG_INFO << "step in [thread id = " << thread_id << ", target id = " << target_id << "]";
    if (granularity == dap::stepping_granularity::instruction) {
        child().step_inst_in(thread_id);
    } else {
        child().step_in(thread_id, target_id);
    }
}


std::vector<dap::step_in_target> server_impl::step_in_targets(size_t frame_id) {
    return {};
}


void server_impl::step_out(size_t thread_id, dap::stepping_granularity granularity) {
    // ignore stepping granularity for step out
    child().step_out(thread_id);
}


void server_impl::restart() {
}


void server_impl::terminate(bool restart) {
    CXXDBG_DAP_LOG_INFO << "terminate [restart = " << restart << "]";
    child().terminate();
}


std::vector<dap::thread> server_impl::threads() {
    return child().threads();
}


std::vector<dap::variable> server_impl::variables(size_t var_ref_id,
                                                  dap::variable_filter filter,
                                                  size_t start,
                                                  size_t count,
                                                  const dap::value_format & fmt) {
    return child().variables(var_ref_id, filter, start, count, fmt);
}


/// Processes disassemble request. Default implementation throws not_implemented_error
std::vector<dap::disassembled_instruction> server_impl::disassemble(std::wstring_view mem_ref,
                                                                    int64_t offset,
                                                                    int64_t inst_offset,
                                                                    uint64_t inst_count,
                                                                    bool resolve_symbols) {

    // parsing address
    uint64_t addr = 0;
    if (!util::parse_hex_uint(util::convert::to_string(std::wstring{mem_ref}), addr)) {
        throw std::runtime_error{"invalid memory address"};
    }

    // adding address offset
    addr += offset;

    // disassembling
    return child().disassemble(addr, inst_offset, inst_count, resolve_symbols);
}


void server_impl::prepare_child() {
    child().disconnected.connect([this] {
        notify_exited();
    });

    child().exited.connect([this](int ecode) {
        send_exited(ecode);
        send_terminated(false);
    });

    child().stopped.connect([this](const std::string & reason,
                                   const std::string & desc,
                                   size_t thread_id,
                                   bool preserve_focus,
                                   const std::string & text,
                                   bool all_threads_stopped,
                                   const std::filesystem::path & src,
                                   size_t line,
                                   size_t column) {
        send_stopped(reason, desc, thread_id, preserve_focus, text, all_threads_stopped, src, line, column);
    });

    child().stdout_received.connect([this](const std::string & data) {
        send_output("stdout", data);
    });

    child().stderr_received.connect([this](const std::string & data) {
        send_output("stderr", data);
    });

    child().module_load_started.connect([this](auto && path) {
        auto msg = std::string{"Loading module "} + path.string() + "...";
        send_progress_start("moduleload-" + path.string(), "Loading module", false, msg);
    });

    child().module_load_finished.connect([this](auto && path) {
        send_progress_end("moduleload-" + path.string());
    });
}


child_process & server_impl::child() {
    if (!child_) {
        throw std::runtime_error("child process does not exist");
    }

    return *child_;
}


}
