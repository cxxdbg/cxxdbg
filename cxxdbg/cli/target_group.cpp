// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file target_group.cpp
/// Contains implementation of the target_group class.

#include "target_group.hpp"
#include "exec_command.hpp"
#include "po_command.hpp"
#include "target_processor.hpp"
#include "cxxdbg/dbg/launch_options.hpp"
#include "cxxdbg/async/async_execution_queue.hpp"
#include "cxxdbg/util/command_line_parser.hpp"


namespace fs = std::filesystem;
namespace po = boost::program_options;


namespace cxxdbg::dbg::cli {


static const std::string target_command_desc = "A set of commands for operating on debugger targets";


target_group::target_group(target_processor & targ_proc, exec_processor & app_proc):
command_group{target_command_desc},
targ_proc_{targ_proc},
app_proc_{app_proc} {

    // target create command

    po::options_description create_opt_desc;
    create_opt_desc.add_options()
            ("exec", po::value<fs::path>(), "Executable file")
            ("work-dir", po::value<fs::path>(), "Work directory");

    auto create_handler_func = [this, create_opt_desc](const auto & cmd, const auto & chandler) {
        // parsing --args options
        std::vector<std::string> opts;
        std::vector<std::string> arg_opts;
        util::parse_args_command_line(cmd.begin(),
                                      cmd.end(),
                                      opts,
                                      arg_opts);

        // parsing opions
        po::positional_options_description pos_desc;
        pos_desc.add("exec", 1);
        po::variables_map var_map;
        po::command_line_parser parser(opts);
        parser.options(create_opt_desc).positional(pos_desc);
        po::store(parser.run(), var_map);

        // checking options

        if (var_map.count("exec") == 0 && arg_opts.empty()) {
            throw std::runtime_error("executable name not specified");
        }

        if (var_map.count("exec") > 0 && !arg_opts.empty()) {
            throw std::runtime_error("both exec and --args specified");
        }

        // loading executable
        fs::path work_dir = var_map.count("work-dir") > 0 ?
                            var_map["work-dir"].as<fs::path>() :
                            fs::current_path();

        std::vector<std::string> cmd_args;

        fs::path exec;
        if (var_map.count("exec") > 0) {
            exec = var_map["exec"].as<fs::path>();
            std::copy(arg_opts.begin(), arg_opts.end(), std::back_inserter(cmd_args));
        } else {
            assert(!arg_opts.empty() && "--args is empty");
            exec = arg_opts.front();
            std::copy(arg_opts.begin() + 1, arg_opts.end(), std::back_inserter(cmd_args));
        }

        targ_proc_.load_target(exec, work_dir, cmd_args, [chandler](const auto & res) {
            if (res.is_ok()) {
                chandler("target loaded");
            } else {
                chandler(res.error());
            }
        });
    };

    std::ostringstream create_help_msg;
    create_help_msg << "Load target\n\n";
    create_help_msg << "Command syntax: target create [options] [--args arguments]\n\n";
    create_help_msg << "Command options:\n";
    create_help_msg << create_opt_desc << "\n";

    auto create_handler = function_command::make(create_handler_func);
    create_handler->set_desc("Load target");
    create_handler->set_help_msg(create_help_msg.str());
    reg_cmd("create", create_handler);


    // target delete command

    auto close_handler = [this](const po::variables_map & vars, const auto & chandler) {
        assert(vars.count("target-index") > 0 && "target-index is not set");
        if (vars["target-index"].as<unsigned int>() != 0) {
            throw std::runtime_error("invalid target index");
        }

        targ_proc_.close_target([chandler](const auto & res) {
            if (res.is_ok()) {
                chandler("target closed");
            } else {
                chandler(res.error());
            }
        });
    };

    auto handler = make_po_command(close_handler);
    handler->set_desc_help("Close target");
    handler->opt_desc().add_options()
            ("target-index", po::value<unsigned int>()->required(), "Index of target");
    handler->popt_desc().add("target-index", 1);

    reg_cmd("delete", handler);


    // redirecting commands to cxxdbg
    reg_app_exec_cmd("list", "List all target");
    reg_app_exec_cmd("modules", "A set of commands for accessing module information");
    reg_app_exec_cmd("select", "Select current target");
    reg_app_exec_cmd("stop-hook", "A set of commands for operating on debugger target stop-hooks");
    reg_app_exec_cmd("symbols", "A set of commands for adding and managing debug symbol files");
    reg_app_exec_cmd("variable", "Read global variable");
}


target_group::~target_group() {
}


void target_group::reg_app_exec_cmd(const std::string & name, const std::string & desc) {
    reg_cmd(name, exec_command::make(app_proc_, std::string("target ") + name, desc));
}


}
