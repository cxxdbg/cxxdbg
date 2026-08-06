// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file debug_child_process.cpp
/// Contains implementation of the debug_child_process class.

#include "debug_child_process.hpp"
#include "log.hpp"
#include "cxxdbg/dbg/core/address_breakpoint.hpp"
#include "cxxdbg/dbg/core/function_breakpoint.hpp"
#include "cxxdbg/dbg/core/regex_lists_defaults.hpp"
#include "cxxdbg/dbg/core/source_position.hpp"
#include "cxxdbg/dbg/core/source_position_breakpoint.hpp"
#include "cxxdbg/log/log.hpp"
#include "dbgfmt/format_options.hpp"
#include "cxxdbg/util/convert.hpp"
#include "cxxdbg/util/print.hpp"
#include <exception>
#include <stdexcept>


namespace fs = std::filesystem;


namespace cxxdbg::dbg::dapsrv {


void debug_child_process::prepare_launch(const path_t & exe_path,
                                         const path_t & work_dir,
                                         const std::vector<std::wstring> & args,
                                         const std::map<std::wstring, std::wstring> & env,
                                         const json_t & json_parameters,
                                         const source_breakpoints_map & source_breakpoints,
                                         const function_breakpoints_vector & function_breakpoints,
                                         const instruction_breakpoints_vector & instruction_breakpoints,
                                         const std::vector<std::wstring> & exception_breakpoint_filters) {

    // saving common launch parameters
    work_dir_ = work_dir;
    args_ = args;
    env_ = env;

    // processing common debug parameters
    process_common_debug_parameters(json_parameters);

    // loading target
    load_target(exe_path);

    // preparing taget after load
    prepare_target(source_breakpoints, function_breakpoints, instruction_breakpoints, exception_breakpoint_filters);
}


void debug_child_process::process_configuration_done(const std::function<void (const std::string &)> & comp_handler) {
    if (was_attached()) {
        // resuming attached process
        CXXDBG_DAP_LOG_INFO << "resuming attached process";
        targ().run();

        // executing completion handler
        comp_handler({});
    } else {
        // setting number of skip stops to 1 for launch from lldb
        set_num_skip_init_stops(1);

        // launching target from lldb

        std::vector<std::string> s_args;
        std::transform(args_.begin(), args_.end(), std::back_inserter(s_args), [](auto && arg) {
            return util::convert::to_string(arg);
        });

        std::map<std::string, std::string> s_env;
        for (auto && entry : env_) {
            s_env.emplace(util::convert::to_string(entry.first), util::convert::to_string(entry.second));
        }

        auto res = targ().launch(work_dir_, s_args, s_env);

        // Setting launching flag. The flag is used in state changed signal processing to
        // ignore state changed events while process is launching.
        is_launching_ = true;

        // we should clear is_launching_ flag and call completion handler
        // after all events posted by launch process were processed and ignored, so we do that in
        // action posted via the post_lldb_event function of the debugger
        dbg_.post_lldb_event([this, comp_handler, res] {
            // clearing launching flag
            is_launching_ = false;

            // executing configuration done completion hanler
            CXXDBG_LOG_SCAT_DEBUG(appcore, target) << "executing configuration done completion handler";
            comp_handler({});

            // setting target state to stopped if it was stopped during launch process
            if (is_stopped_after_launching_) {
                on_target_state_changed(core::target::state_t::stopped);

                // clearing is_stopped_after_launching_ flag
                is_stopped_after_launching_ = false;
            }
        });
    }
}


void debug_child_process::attach(size_t pid,
                                 const std::wstring & program,
                                 const json_t & json_parameters,
                                 const source_breakpoints_map & source_breakpoints,
                                 const function_breakpoints_vector & function_breakpoints,
                                 const instruction_breakpoints_vector & instruction_breakpoints,
                                 const std::vector<std::wstring> & exception_breakpoint_filters) {
    // processing common parameters
    process_common_debug_parameters(json_parameters);
    
    // attaching to target
    if (program.empty()) {
        attach_pid_ = pid;
        targ_ = dbg_.attach(attach_pid_);
    } else {
        attach_pid_ = 0;
        targ_ = dbg_.attach(util::convert::to_string(program));
    }

    // preparing taget after attach
    prepare_target(source_breakpoints, function_breakpoints, instruction_breakpoints, exception_breakpoint_filters);
}


std::vector<dap::thread> debug_child_process::threads() {
    if (targ().state() != core::target::state_t::stopped &&
        targ().state() != core::target::state_t::running) {

        CXXDBG_DAP_LOG_ERROR << "can't get threads: target is not launched";
        throw std::runtime_error{"can't get threads: target is not launched"};
    }

    std::vector<dap::thread> res;
    for (auto && thrd : targ().threads()) {
        std::wostringstream thrd_name;
        thrd_name << L"thread " << thrd.id();
        res.push_back({thrd.id(), thrd_name.str()});
    }

    return res;
}


size_t debug_child_process::stack_trace(size_t thread_id,
                                        size_t first,
                                        size_t max_cnt,
                                        const dap::stack_frame_format & fmt,
                                        std::vector<dap::stack_frame> & res) {

    auto thrd = thread_by_id(thread_id);

    size_t last = max_cnt == SIZE_MAX ? SIZE_MAX : first + max_cnt;

    for (size_t i = first; i < last; ++i) {
        auto frm = thrd.frame(i);
        if (!frm.is_valid()) {
            break;
        }

        CXXDBG_DAP_LOG_TRACE << "Frame " << i << " CFA: " << frm.get_cfa() << ", PC: " << frm.read_pos().load_addr();

        // adding stack frame into maps if does not exist
        size_t frame_id = 0;
        auto it = frame_ids_.find(frm);
        if (it == frame_ids_.end()) {
            frame_id = next_stack_frame_id_++;
            frames_.emplace(frame_id, frm);
            frame_ids_.emplace(frm, frame_id);
        } else {
            frame_id = it->second;
        }

        // reading frame position
        auto code_pos = frm.read_pos();

        // for non first frames we have to offset code position by -1
        if (i != 0) {
            code_pos = code_pos.offset(-1);
        }

        // reading frame source position
        fs::path src;
        size_t line = 0;
        if (code_pos.get_src_pos().is_valid()) {
            src = code_pos.get_src_pos().path();
            line = code_pos.get_src_pos().line();
        }

        auto fname = code_pos.get_func_name();

        // TODO: read parameters if specified in format options

        auto pres_hint = dap::stack_frame_presentation_hint::normal;
        if (std::regex_search(fname, group_regex_)) {
            pres_hint = dap::stack_frame_presentation_hint::subtle;
        }

        // Workaround for stops on assertion failure. Don't send source positions
        // for assertion failure functions
        // to client because VS code displays errors if stop possition is in
        // source that does not exist.
        const std::string assert_fail_prefix = "__assert_fail";
        if (fname == "__GI_raise" ||
            fname == "__GI_abort" ||
            fname.substr(0, assert_fail_prefix.size()) == assert_fail_prefix ||
            fname == "__GI___assert_fail") {
            src = fs::path{};
            line = 0;
        }

        // getting instruction pointer address for stack frame
        std::ostringstream inst_ptr_ref_str;
        util::print_hex(inst_ptr_ref_str, frm.read_pos().load_addr(), targ().ptr_byte_size(), targ().ptr_byte_size());

        dap::stack_frame dap_frame {
            frame_id,
            std::wstring{fname.begin(), fname.end()},
            src,
            line,
            1,
            SIZE_MAX,
            SIZE_MAX,
            util::convert::to_wstring(inst_ptr_ref_str.str()),
            {},
            pres_hint
        };

        res.push_back(dap_frame);
    };

    // detecting total number of frames that we will send to client
    size_t total_frames = SIZE_MAX;
    if (max_cnt == SIZE_MAX || res.size() < max_cnt) {
        // we have read all frames and know number of frames in call stack
        total_frames = first + res.size();
    } else {
        // we don't know real number of frames in call stack. DAP specification says
        // that total number of frames may be ommited to force client request more frames
        // but it does not work with vscode. So we return very large number here
        total_frames = first + res.size() + 16777216;
    }

    // we need return number of frames available
    return total_frames;
}


std::vector<dap::scope> debug_child_process::scopes(size_t frame_id) {
    // checking that stack frame exists
    auto it = frames_.find(frame_id);
    if (it == frames_.end()) {
        std::ostringstream msg;
        msg << "Stack frame with ID " << frame_id << " not found";
        throw std::runtime_error{msg.str()};
    }

    auto frame = it->second;
    std::vector<dap::scope> res;

    // Locals scope
    auto locals = vars_->locals_ref(frame);
    res.push_back({
        L"Locals",
        L"locals",
        locals->id(),
        locals->vars_size()
    });

    // Registers scope
    auto registers = vars_->registers_ref(frame);
    res.push_back({
        L"Registers",
        L"registers",
        registers->id(),
        registers->vars_size()
    });

    return res;
}


std::vector<dap::variable> debug_child_process::variables(size_t var_ref_id,
                                                          dap::variable_filter filter,
                                                          size_t start,
                                                          size_t count,
                                                          const dap::value_format & fmt) {
    // getting variable reference object for specified id
    auto var_ref = vars_->var_ref(var_ref_id);

    // setting format options in target
    auto opts = fmt_opts_;
    if (fmt.hex) {
        opts.set_hex(true);
    }
    targ_->set_fmt_opts(opts);

    // building list of variables
    std::vector<dap::variable> res;
    for (size_t i = 0, e = var_ref->vars_size(); i < e; ++i) {
        auto fres = var_ref->var_at(i);

        std::wstring mem_ref;
        if (fres.address() != UINT64_MAX) {
            std::wostringstream str;
            util::print_hex(str, fres.address(), sizeof(uint64_t), targ().ptr_byte_size());
            mem_ref = str.str();
        }

        dap::variable dvar {
            fres.name(),
            fres.val(),
            fres.type(),
            {},
            {},
            fres.childs_size() > 0 ? vars_->fmt_result_ref(fres.res())->id() : 0,
            fres.childs_size() > 0 ? fres.childs_size() : SIZE_MAX,
            SIZE_MAX,
            mem_ref
        };
        res.push_back(dvar);
    }

    return res;
}


dap::evaluate_response debug_child_process::evaluate(const std::wstring & expr,
                                                     size_t frame_id,
                                                     const std::wstring & context,
                                                     const dap::value_format & fmt) {
    // getting frame by id
    auto it = frames_.find(frame_id);
    if (it == frames_.end()) {
        std::ostringstream msg;
        msg << "Stack frame with ID " << frame_id << " not found";
        throw std::runtime_error{msg.str()};
    }

    // setting format options in target
    auto opts = fmt_opts_;
    if (fmt.hex) {
        opts.set_hex(true);
    }
    targ_->set_fmt_opts(opts);

    auto frame = it->second;
    auto val = frame.eval_expr(std::string{expr.begin(), expr.end()});
    auto fres = targ().format_value(val);

    std::wstring mem_ref;
    if (fres.address() != UINT64_MAX) {
        std::wostringstream str;
        util::print_hex(str, fres.address(), sizeof(uint64_t), targ().ptr_byte_size());
        mem_ref = str.str();
    }

    return {
        fres.val(),
        fres.type(),
        {},
        fres.childs_size() > 0 ? vars_->fmt_result_ref(fres)->id() : 0,
        fres.childs_size() > 0 ? fres.childs_size() : SIZE_MAX,
        SIZE_MAX,
        mem_ref
    };
}


void debug_child_process::set_breakpoints(const path_t & src,
                                          const std::vector<dap::source_breakpoint> & bpts,
                                          const std::vector<size_t> & line_numbers,
                                          bool source_modified) {
    // removing all breakpoints in this source
    
    std::vector<core::breakpoint::id_t> bps_to_remove;
    for (auto && bp : targ().breakpoints()) {
        if (auto sp_bp = dynamic_cast<core::source_position_breakpoint*>(bp)) {
            if (sp_bp->pos().path() == src) {
                bps_to_remove.push_back(sp_bp->id());
            }
        }
    }

    for (auto && bp_id : bps_to_remove) {
        CXXDBG_DAP_LOG_INFO << "removing breakpoint " << bp_id.num();
        targ().remove_breakpoint(targ().get_breakpoint_by_id(bp_id));
    }


    // adding new breakpoints
    for (auto && src_bp : bpts) {
        add_source_breakpoint(src, src_bp);
    }
}


void debug_child_process::set_exception_breakpoints(const std::vector<std::wstring> & filters,
                                                    const std::vector<dap::exception_options> & opts) {
    set_eh_breakpoints(filters);
}


void debug_child_process::set_function_breakpoints(const std::vector<dap::function_breakpoint> & bpts) {
    // removing all function breakpoints
    
    std::vector<core::breakpoint::id_t> bps_to_remove;
    for (auto && bp : targ().breakpoints()) {
        if (auto code_bp = dynamic_cast<core::function_breakpoint*>(bp)) {
            bps_to_remove.push_back(code_bp->id());
        }
    }

    for (auto && bp_id : bps_to_remove) {
        CXXDBG_DAP_LOG_INFO << "removing breakpoint " << bp_id.num();
        targ().remove_breakpoint(targ().get_breakpoint_by_id(bp_id));
    }


    // adding new breakpoints
    for (auto && func_bp : bpts) {
        add_function_breakpoint(func_bp);
    }
}


void debug_child_process::set_instruction_breakpoints(const std::vector<dap::instruction_breakpoint> & bpts) {
    // removing all function breakpoints
    
    std::vector<core::breakpoint::id_t> bps_to_remove;
    for (auto && bp : targ().breakpoints()) {
        if (auto code_bp = dynamic_cast<core::address_breakpoint*>(bp)) {
            bps_to_remove.push_back(code_bp->id());
        }
    }

    for (auto && bp_id : bps_to_remove) {
        CXXDBG_DAP_LOG_INFO << "removing breakpoint " << bp_id.num();
        targ().remove_breakpoint(targ().get_breakpoint_by_id(bp_id));
    }


    // adding new breakpoints
    for (auto && inst_bp : bpts) {
        add_instruction_breakpoint(inst_bp);
    }
}


bool debug_child_process::process_continue(size_t thread_id) {
    prepare_to_run();
    targ().run();
    return true;
}


void debug_child_process::pause(size_t thread_id) {
    requested_pause_thread_id_ = thread_id;
    targ().stop();
}


void debug_child_process::next(size_t thread_id) {
    prepare_to_run();
    set_current_thread(thread_id);
    targ().step_over(thread_id, avoid_regex_);
}


void debug_child_process::next_inst(size_t thread_id) {
    prepare_to_run();
    set_current_thread(thread_id);
    targ().step_inst_over(thread_id);
}


void debug_child_process::step_in(size_t thread_id, size_t target_id) {
    prepare_to_run();
    set_current_thread(thread_id);
    targ().step_into(thread_id, avoid_nodebug_, "", avoid_regex_, step_through_regex_);
}


void debug_child_process::step_inst_in(size_t thread_id) {
    prepare_to_run();
    set_current_thread(thread_id);
    targ().step_inst_into(thread_id);
}


void debug_child_process::step_out(size_t thread_id) {
    prepare_to_run();
    set_current_thread(thread_id);
    targ().step_out(thread_id, avoid_nodebug_, step_through_regex_);
}


void debug_child_process::disconnect(bool terminate) {
    CXXDBG_DAP_LOG_TRACE << "client_term_debug_child_process: disconnect requested";

    // if target is not loaded or not launched then just notify server that we want exit
    if (targ().state() == core::target::state_t::loaded) {
        CXXDBG_DAP_LOG_TRACE << "target state is loaded, sending disconnect notification";
        disconnected();
        return;
    }

    // setting disconnect_request_ flag. It will be checked in state change handler
    disconnect_requested_ = true;

    // if target is running then stop it first
    if (targ().state() == core::target::state_t::running) {
        CXXDBG_DAP_LOG_DEBUG << "stopping target before termination for disconnect";
        targ().stop();
        return;
    }

    // terminate target only if target was launched (not attached to) or terminate flag is true
    if (attach_pid_ == SIZE_MAX || terminate) {
        assert(targ().state() == core::target::state_t::stopped && "invalid target state");
        CXXDBG_DAP_LOG_DEBUG << "terminating target for disconnect";
        targ().terminate();
    } else {
        targ().detach();   
    }
}


void debug_child_process::terminate() {
    // if target is not launched then doing nothing
    if (targ().state() == core::target::state_t::loaded) {
        return;
    }

    terminate_requested_ = true;

    // if target is running then interrupt it first. Target termination will be started
    // in change state event handler
    if (targ().state() == core::target::state_t::running) {
        CXXDBG_DAP_LOG_DEBUG << "interrupting target for terminate";
        targ().stop();
        return;
    }

    CXXDBG_DAP_LOG_DEBUG << "terminating target for terminate, target state = " << static_cast<int>(targ().state());
    assert(targ().state() == core::target::state_t::stopped && "invalid target state");
    targ().terminate();
}


std::vector<dap::disassembled_instruction>
debug_child_process::disassemble(uint64_t addr, int64_t inst_offset, size_t inst_count, bool resolve_symbols) {
    // disassembling
    auto insts = targ().disassemble(addr, inst_offset, inst_count);

    // converting instructions
    std::vector<dap::disassembled_instruction> res;
    for (const auto & inst : insts) {
        dap::disassembled_instruction dap_inst{
            util::convert::to_wstring(inst.addr_str(targ().ptr_byte_size())),
            util::convert::to_wstring(inst.data_str()),
            util::convert::to_wstring(inst.string()),
            util::convert::to_wstring(inst.symbol())
        };

        if (inst.src_pos().is_valid()) {
            dap_inst.source = inst.src_pos().path();
            dap_inst.line = inst.src_pos().line();
        }

        res.push_back(dap_inst);
    }

    return res;
}


dap::read_memory_result debug_child_process::read_memory(std::wstring_view mem_ref, int64_t offset, size_t size) {
    // parsing memory address
    uint64_t addr;
    if (!util::parse_hex_uint(util::convert::to_string(std::wstring{mem_ref}), addr)) {
        std::ostringstream msg;
        msg << "invalid memory address: " << util::convert::to_string(std::wstring{mem_ref});
        throw std::runtime_error{msg.str()};
    }

    constexpr size_t step = 64;
    auto end_addr = addr + size;

    // reading memory
    auto curr_addr = addr;
    std::vector<char> data;
    data.reserve(size);
    while (true) {
        auto block_size = step;
        if (block_size > end_addr - curr_addr) {
            block_size = end_addr - curr_addr;
        }

        // reading data block
        auto block_data = targ().read_memory(curr_addr, block_size);

        // adding block to data
        std::copy(block_data.begin(), block_data.end(), std::back_inserter(data));

        if (block_data.size() != block_size) {
            // not all memory can be read
            break;
        }

        curr_addr += block_size;

        // checking for the end of requested region
        assert(curr_addr <= end_addr && "invalid current address");
        if (curr_addr == end_addr) {
            break;
        }
    }

    // detecting number of unreadable bytes if not all memory was read
    size_t unreadable_bytes = 0;
    if (data.size() != size) {
        assert(data.size() < size && "invalid data size");

        curr_addr = addr + data.size();
        while (true) {
            if (curr_addr > end_addr - step) {
                // end of requested range reached
                unreadable_bytes = size - data.size();
                break;
            }

            // trying read next memory block
            curr_addr += step;

            if (targ().read_memory(curr_addr, step).size() == step) {
                // memory block can be read
                unreadable_bytes = curr_addr - addr - data.size();
                break;
            }
        }
    }

    std::wostringstream res_addr_str;
    util::print_hex(res_addr_str, addr, sizeof(addr), targ().ptr_byte_size());

    return dap::read_memory_result{res_addr_str.str(), unreadable_bytes, data};
}


void debug_child_process::load_target(const path_t & exe_path) {
    CXXDBG_DAP_LOG_INFO << "loading target " << exe_path;
    targ_ = dbg_.load_target(exe_path);
}


core::target & debug_child_process::targ() {
    if (!targ_) {
        throw std::runtime_error{"target does not exist"};
    }

    return *targ_;
}


void debug_child_process::prepare_target(const source_breakpoints_map & source_breakpoints,
                                         const function_breakpoints_vector & function_breakpoints,
                                         const instruction_breakpoints_vector & instruction_breakpoints,
                                         const std::vector<std::wstring> & exception_breakpoint_filters) {
    // creating variable reference table
    vars_ = std::make_unique<variable_reference_table>(*targ_);

    // listening for target state change signals
    targ().connect_state_changed([this](auto st) {
        on_target_state_changed(st);
    });

    // listening for target stdout
    targ().stdout_received().connect([this](auto && output) {
        stdout_received(output);
    });

    // listening for target stderr
    targ().stderr_received().connect([this](auto && output) {
        stderr_received(output);
    });

    // listening for modules load signal
    targ().modules_loaded.connect([this](auto && mods) {
        on_modules_loaded(mods);
    });

    // installing breakpoints
    install_breakpoints(source_breakpoints, function_breakpoints, instruction_breakpoints, exception_breakpoint_filters);
}



void debug_child_process::process_common_debug_parameters(const json_t & parameters) {
    avoid_nodebug_ = parameters.value("avoidNodebug", true);

    // calculating avoid regex
    
    bool avoid_std_functions = parameters.value("avoidStdFunctions", true);
    auto custom_avoid_list = parameters.value("avoidRegexList", std::vector<std::string>{});

    core::regex_lists avoid_list;

    if (avoid_std_functions) {
        avoid_list = core::functions_to_skip::default_value();
    }

    core::regex_list_item custom_avoid_item("User defined functions to avoid", true);
    for (auto && r : custom_avoid_list) {
        custom_avoid_item.add(r);
    }

    avoid_list.add(custom_avoid_item);
    avoid_regex_ = avoid_list.combine_regex();


    // calculating step through regex
    
    bool step_through_std_functions = parameters.value("stepThroughStdFunctions", true);
    auto custom_step_through_list = parameters.value("stepThroughRegexList", std::vector<std::string>{});

    core::regex_lists step_through_list;

    if (step_through_std_functions) {
        step_through_list = core::functions_to_step_into::default_value();
    }

    core::regex_list_item custom_step_through_item("User defined functions to step through", true);
    for (auto && r : custom_step_through_list) {
        custom_step_through_item.add(r);
    }

    step_through_list.add(custom_step_through_item);
    step_through_regex_ = step_through_list.combine_regex();


    // calculating group regex

    bool group_std_functions = parameters.value("markStdFunctions", true);
    auto custom_group_list = parameters.value("markRegexList", std::vector<std::string>());

    core::regex_lists group_list;

    if (group_std_functions) {
        group_list = core::functions_to_group::default_value();
    }

    core::regex_list_item custom_group_item("User defined functions to group", true);
    for (auto && r : custom_group_list) {
        custom_group_item.add(r);
    }

    group_list.add(custom_group_item);
    group_regex_ = std::regex{group_list.combine_regex()};


    // processing format options
    fmt_opts_.set_hex(parameters.value("displayHex", fmt_opts_.hex()));
    fmt_opts_.set_min_hex_size(parameters.value("displayMinHexSize", fmt_opts_.min_hex_size()));
    fmt_opts_.set_show_ptr_addr(parameters.value("displayPointerAddresses", fmt_opts_.show_ptr_addr()));
    fmt_opts_.set_show_raw_data(parameters.value("displayRawData", fmt_opts_.show_raw_data()));
    fmt_opts_.set_show_rec_addr(parameters.value("displayStructAddresses", fmt_opts_.show_rec_addr()));


    // executing initialization commands
    auto cmds = parameters.value("initCommands", std::vector<std::string>{});
    for (auto && cmd : cmds) {
        CXXDBG_DAP_LOG_INFO << "executing init command: " << cmd;
        auto res = dbg_.execute(cmd);
        CXXDBG_DAP_LOG_INFO << "result of command execution: " << res;
    }


    // setting executable search paths
    auto exec_search_paths = parameters.value("execSearchPaths", std::vector<std::string>{});
    if (!exec_search_paths.empty()) {
        std::ostringstream paths_str;
        bool first = true;
        for (auto && path : exec_search_paths) {
            if (!first) {
                paths_str << ' ';
            } else {
                first = false;
            }

            paths_str << '\"' << path << '\"';
        }

        CXXDBG_DAP_LOG_INFO << "setting target.exec-search-paths to '" << paths_str.str() << "'";
        dbg_.set_settings_value("target.exec-search-paths", paths_str.str());
    }


    // connecting to platform if specified in parameters

    auto platform_name = parameters.value("platform", std::string{});
    auto platform_url = parameters.value("platformUrl", std::string{});

    if (platform_name.empty()) {
        // not platform is specified
        return;
    }

    // appending "connect://" prefix to platform url
    if (platform_url.find("://") == std::string::npos) {
        platform_url = std::string{"connect://"} + platform_url;
    }

    // connecting to platform
    dbg_.connect_to_platform(platform_name, platform_url);
}


void debug_child_process::on_target_state_changed(core::target::state_t st) {
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

    if (st == core::target::state_t::loaded) {
        CXXDBG_DAP_LOG_INFO << "target state was changed to loaded";

        // changing state to loaded means that target was exited or terminated

        // notifying client that target exited
        exited(targ().exit_code());

        // notifying server that disconnect is done if disconnect was requested
        if (disconnect_requested_) {
            CXXDBG_DAP_LOG_DEBUG << "target exited for disconnect, disconnecting";
            disconnected();
        }

    } else if (st == core::target::state_t::launching) {
        CXXDBG_DAP_LOG_INFO << "target state was changed to launching";
    } else if (st == core::target::state_t::running) {
        CXXDBG_DAP_LOG_INFO << "target state was changed to running";
    } else if (st == core::target::state_t::stopped) {
        CXXDBG_DAP_LOG_INFO << "target state was changed to stopped";

        // skipping initial stop
        if (num_skip_init_stops_ != 0 && attach_pid_ == SIZE_MAX) {
            CXXDBG_DAP_LOG_INFO << "skipping initial stop: " << num_skip_init_stops_;
            --num_skip_init_stops_;
            targ().run();
            return;
        }

        // checking disconnect or terminate flag
        if (disconnect_requested_ && attach_pid_ != SIZE_MAX) {
            CXXDBG_DAP_LOG_DEBUG << "target interrupted, detaching for disconnect";
            targ().detach();
            return;
        } else if (disconnect_requested_ || terminate_requested_) {
            // if disconnect or terminate is requested then we should terminate target after
            // stop and wait for its termination
            if (disconnect_requested_) {
                CXXDBG_DAP_LOG_DEBUG << "target interrupted, terminating for disconnect";
            } else {
                CXXDBG_DAP_LOG_DEBUG << "target interrupted, terminating for terminate";
            }
            targ().terminate();
            return;
        }

        std::string stop_reason;
        std::ostringstream desc;

        CXXDBG_DAP_LOG_DEBUG << "checking process stop reason...";

        // processing stop reason
        switch (targ().stop_reason()) {
        case core::stop_reason_t::unknown:
            stop_reason = "unknown";
            desc << "Process stopped by unknown reason";
            break;
        case core::stop_reason_t::trace:
            stop_reason = "step";
            desc << "Process stopped after step";
            break;
        case core::stop_reason_t::breakpoint:
        {
            desc << "Process stopped on breakpont";

            // we need set correct stop reason for different breakpoints

            // use breakpoint as default
            stop_reason = "breakpoint";

            auto bp_num = targ().active_bp_num();
            if (bp_num != 0) {
                if (auto bp = targ().get_breakpoint_by_id({core::breakpoint_type::code, bp_num})) {
                    if (dynamic_cast<core::function_breakpoint*>(bp)) {
                        stop_reason = "function breakpoint";
                    } else if (dynamic_cast<core::address_breakpoint*>(bp)) {
                        stop_reason = "instruction breakpoint";
                    }
                }
            }

            break;
        }
        case core::stop_reason_t::watchpoint:
            stop_reason = "data breakpoint";
            desc << "Process stopped on watchpoint";
            break;
        case core::stop_reason_t::signal:
            // checking if we stopped after pause request
            if (requested_pause_thread_id_ != SIZE_MAX) {
                stop_reason = "pause";
                desc << "Process paused";
            } else {
                stop_reason = "signal";
                desc << "Process stopped on signal " << targ().signal_num();
            }
            break;
        case core::stop_reason_t::exec:
            stop_reason = "exec";
            desc << "Process stopped on exec";
            break;
        case core::stop_reason_t::crashed:
            stop_reason = "crashed";
            desc << "Process crashed";
            break;
        default:
            assert(false && "unknown stop reason");
        }

        // selecting stop thread
        
        std::unique_ptr<core::thread> thrd;

        // setting stop thread to requested pause thread if it's found
        if (requested_pause_thread_id_ != SIZE_MAX) {
            CXXDBG_DAP_LOG_DEBUG << "requested pause thread id " << requested_pause_thread_id_;
            thrd = try_thread_by_id(requested_pause_thread_id_);

            if (thrd) {
                CXXDBG_DAP_LOG_DEBUG << "requested pause thread " << requested_pause_thread_id_ << " found";
            } else {
                CXXDBG_DAP_LOG_INFO << "requested pause thread " << requested_pause_thread_id_ << " not found";
            }
        }

        // setting stop thread to stop reason thread if it's not yet set
        if (!thrd) {
            CXXDBG_DAP_LOG_DEBUG << "setting stop thread to stop reason thread";
            if (targ().stop_reason_thread_index() != SIZE_MAX) {
                thrd = std::make_unique<core::thread>(targ().stop_reason_thread());
            } else {
                thrd = std::make_unique<core::thread>(targ().thread_at(0));
            }
        }

        CXXDBG_DAP_LOG_DEBUG << "stop reason thread " << thrd->id();


        requested_pause_thread_id_ = SIZE_MAX;

        fs::path src;
        size_t line = SIZE_MAX;

        // getting current frame position
        auto cframe = thrd->frame(0);
        if (cframe.is_valid()) {
            auto pos = cframe.read_pos().get_src_pos();
            if (pos.is_valid()) {
                src = pos.path();
                line = pos.line();
            }
        }

        // notifying client that process was stopped
        stopped(stop_reason,
                        desc.str(),
                        thrd->id(),
                        false,
                        "",
                        true,
                        src,
                        line,
                        1);
    } else {
        assert(false && "unknown core target state");
    }
}


void debug_child_process::prepare_to_run() {
    if (targ().state() != core::target::state_t::stopped) {
        throw std::runtime_error{"target is not in stopped state"};
    }

    frames_.clear();
    frame_ids_.clear();
    next_stack_frame_id_ = 1;
    vars_->clear();
}


void debug_child_process::install_breakpoints(const source_breakpoints_map & source_breakpoints,
                                              const function_breakpoints_vector & function_breakpoints,
                                              const instruction_breakpoints_vector & instruction_breakpoints,
                                              const std::vector<std::wstring> & exception_breakpoint_filters) {

    for (auto && [src, bps] : source_breakpoints) {
        for (auto && [id, src_bp] : bps) {
            auto bp = add_source_breakpoint(src, src_bp);
        }
    }

    for (auto && [id, func_bp] : function_breakpoints) {
        auto bp = add_function_breakpoint(func_bp);
    }

    for (auto && [id, func_bp] : instruction_breakpoints) {
        auto bp = add_instruction_breakpoint(func_bp);
    }

    for (auto && eh_filter : exception_breakpoint_filters) {
        auto bp = add_eh_breakpoint(eh_filter);
    }
}


core::code_breakpoint * debug_child_process::add_source_breakpoint(const std::filesystem::path & src,
                                                                   const dap::source_breakpoint & src_bp) {
    auto bp = targ().add_breakpoint(core::source_position{src, static_cast<unsigned int>(src_bp.line)});
    CXXDBG_DAP_LOG_INFO << "added breakpoint " << bp->id().num() << " at " << src << ":" << src_bp.line;
    setup_breakpoint(bp, src_bp.condition, src_bp.hit_condition, src_bp.log_message);
    return bp;
}


core::code_breakpoint * debug_child_process::add_function_breakpoint(const dap::function_breakpoint & func_bp) {
    std::string bp_name{func_bp.name.begin(), func_bp.name.end()};
    auto bp = targ().add_breakpoint(bp_name);
    CXXDBG_DAP_LOG_INFO << "added breakpoint " << bp->id().num() << " at '" << bp_name << "'";
    setup_breakpoint(bp, func_bp.condition, func_bp.hit_condition, L"");
    return bp;
}


core::code_breakpoint * debug_child_process::add_instruction_breakpoint(const dap::instruction_breakpoint & inst_bp) {
    // parsing address
    uint64_t addr = 0;
    if (!util::parse_hex_uint(util::convert::to_string(inst_bp.inst_reference), addr)) {
        std::ostringstream msg;
        msg << "can't parse address for instruction breakpoint: '"
            << util::convert::to_string(inst_bp.inst_reference) << "'";
        throw std::runtime_error{msg.str()};
    }

    // adding offset
    addr += inst_bp.offset;

    // adding breakpoint
    auto bp = targ().add_breakpoint(addr);
    CXXDBG_DAP_LOG_INFO << "added breakpoint " << bp->id().num() << " at address " << addr;
    setup_breakpoint(bp, inst_bp.condition, inst_bp.hit_condition, L"");
    return bp;
}


core::code_breakpoint * debug_child_process::add_eh_breakpoint(const std::wstring & filter) {
    std::string func_name;
    if (filter == L"thrown") {
        func_name = "__cxa_throw";
    } else if (filter == L"caught") {
        func_name = "__cxa_begin_catch";
    } else {
        std::ostringstream msg;
        msg << "Unknown exception filter: '" << std::string{filter.begin(), filter.end()} << "'";
    }

    auto bp = targ().add_breakpoint(func_name);
    CXXDBG_DAP_LOG_INFO << "added breakpoint " << bp->id().num() << " at '" << func_name << "'";

    if (filter == L"thrown") {
        eh_thrown_bp_ = bp;
    } else if (filter == L"caught") {
        eh_caught_bp_ = bp;
    }

    return bp;
}


void debug_child_process::set_eh_breakpoints(const std::vector<std::wstring> & filters) {
    // removing old EH breakpoints

    if (eh_thrown_bp_ != nullptr) {
        targ().remove_breakpoint(eh_thrown_bp_);
        eh_thrown_bp_ = nullptr;
    }

    if (eh_caught_bp_ != nullptr) {
        targ().remove_breakpoint(eh_caught_bp_);
        eh_caught_bp_ = nullptr;
    }


    // adding new EH breakpoints

    for (auto && f : filters) {
        add_eh_breakpoint(f);
    }
}


void debug_child_process::set_current_thread(size_t thread_id) {
    size_t idx = 0;
    for (auto thrd : targ().threads()) {
        if (thrd.id() == thread_id) {
            CXXDBG_DAP_LOG_DEBUG << "set current thread to " << idx << " (thread id = " << thread_id << ")";
            targ().set_current_thread_index(idx);
            return;
        }

        ++idx;
    }
}


std::unique_ptr<core::thread> debug_child_process::try_thread_by_id(size_t id) {
    for (auto && thrd : targ().threads()) {
        if (thrd.id() == id) {
            return std::make_unique<core::thread>(thrd);
        }
    }

    return {};
}


core::thread debug_child_process::thread_by_id(size_t id) {
    CXXDBG_DAP_LOG_TRACE << "debug_child_process::thread_by_id: id = " << id;

    auto thrd = try_thread_by_id(id);
    if (thrd) {
        return *thrd;
    }

    CXXDBG_DAP_LOG_ERROR << "thread with ID " << id << " not found";
    std::ostringstream msg;
    msg << "thread with id " << id << " not found";
    throw std::runtime_error{msg.str()};
}



void debug_child_process::setup_breakpoint(core::code_breakpoint * bp,
                                           const std::wstring & condition,
                                           const std::wstring & hit_condition,
                                           const std::wstring & log_message) {
    std::string scond{condition.begin(), condition.end()};
    std::string shc{hit_condition.begin(), hit_condition.end()};

    CXXDBG_DAP_LOG_DEBUG << "setup breakpoint: condition = '" << scond << "', hitcount = '" << shc << "'";

    if (!scond.empty()) {
        bp->set_condition(scond);
    }

    if (!shc.empty()) {
        std::istringstream str{shc};
        unsigned int cnt;
        if (str >> cnt) {
            bp->set_ignore_count(cnt);
        }
    }
}


void debug_child_process::on_modules_loaded(const std::vector<core::module> & mods) {
    for (auto && m : mods) {
        auto mod = m;
        module_load_started(mod.path());
        mod.reindex();
        module_load_finished(mod.path());
    }
}


}
