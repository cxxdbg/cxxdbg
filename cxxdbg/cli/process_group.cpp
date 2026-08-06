// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file process_group.cpp
/// Contains implementation of the process_group class.

#include "process_group.hpp"
#include "exec_command.hpp"
#include "function_command.hpp"
#include "po_command.hpp"
#include "process_processor.hpp"
#include "cxxdbg/util/command_line_parser.hpp"
#include <filesystem>
#include <boost/program_options/parsers.hpp>


namespace fs = std::filesystem;
namespace po = boost::program_options;


namespace cxxdbg::dbg::cli {


process_group::process_group(exec_processor & app_proc,
                             process_processor & proc_proc):
command_group{"A set of commands for operating on a process"} {

    reg_attach(proc_proc);
    reg_continue(proc_proc);
    reg_detach(proc_proc);
    reg_interrupt(proc_proc);
    reg_kill(proc_proc);
    reg_launch(proc_proc);

    // app-exec commands
    reg_app_exec_cmd(app_proc, "handle", "Show/modify signal handling");
    reg_app_exec_cmd(app_proc, "load", "Load shared library");
    reg_app_exec_cmd(app_proc, "plugin", "Send custom command to process plugin");
    reg_app_exec_cmd(app_proc, "save-core", "Save core file");
    reg_app_exec_cmd(app_proc, "signal", "Send signal to process");
    reg_app_exec_cmd(app_proc, "status", "Show process status");
    reg_app_exec_cmd(app_proc, "unload", "Unload shared library");
}


void process_group::reg_app_exec_cmd(exec_processor & app_proc,
                                     const std::string & name,
                                     const std::string & desc) {
    reg_cmd(name, exec_command::make(app_proc, std::string("process ") + name, desc));
}


void process_group::reg_attach(process_processor & proc_proc) {

    auto attach_handl = [&proc_proc](const po::variables_map & vars, const auto & chandler) {

        auto attach_chandler = [chandler](const auto & res) {
            chandler(make_result_msg("attached to process", res));
        };

        if (vars.count("target") > 0) {
            auto t = vars["target"].as<std::string>();

            // trying parse target as process ID
            unsigned long pid = 0;
            std::istringstream t_str{t};
            if ((t_str >> pid) && t_str.eof() && pid != 0) {
                proc_proc.attach_pid(pid, attach_chandler);
            } else {
                proc_proc.attach_name(t, attach_chandler);
            }
        } else if (vars.count("pid") > 0) {
            // attaching to process with specified id
            proc_proc.attach_pid(vars["pid"].as<unsigned long>(), attach_chandler);
        } else if (vars.count("name") > 0) {
            // attaching to process with name
            proc_proc.attach_name(vars["name"].as<std::string>(), attach_chandler);
        } else {
            // asking user for PID
            proc_proc.attach(attach_chandler);
        }
    };

    auto h = make_po_command(attach_handl);
    h->set_desc_help("Attach to process");
    h->opt_desc().add_options()
        ("pid", po::value<unsigned long>(), "The process ID of an existing process to attach to")
        ("name", po::value<std::string>(), "The name of the process to attach to")
        ("target", po::value<std::string>(), "ID or name of the process to attach to");
    h->popt_desc().add("target", 1);
    reg_cmd("attach", h);
}


void process_group::reg_continue(process_processor & proc_proc) {

    auto c_handler = [&proc_proc](const auto & var_map, const auto & handler) {
        proc_proc.resume([handler](const auto & res) {
            handler(make_result_msg("continuing process", res));
        });
    };

    auto h = make_po_command(c_handler);
    h->set_desc_help("Continue process execution");
    reg_cmd("continue", h);
}


void process_group::reg_detach(process_processor & proc_proc) {

    auto detach_func = [&proc_proc](const auto & var_map, const auto & chandler) {
        proc_proc.detach([chandler](const auto & res) {
            chandler(make_result_msg("detached from process", res));
        });
    };

    auto h = make_po_command(detach_func);
    h->set_desc_help("Detach from process");
    reg_cmd("detach", h);
}


void process_group::reg_interrupt(process_processor & proc_proc) {
    auto func = [&proc_proc](const auto & var_map, const auto & chandler) {
        proc_proc.interrupt([chandler](const auto & res) {
            chandler(make_result_msg("interrupted process", res));
        });
    };

    auto h = make_po_command(func);
    h->set_desc_help("Interrupt process");
    reg_cmd("interrupt", h);
}


void process_group::reg_kill(process_processor & proc_proc) {
    auto func = [&proc_proc](const auto & var_map, const auto & chandler) {
        proc_proc.kill([chandler](const auto & res) {
            chandler(make_result_msg("killed process", res));
        });
    };

    auto h = make_po_command(func);
    h->set_desc_help("Kill process");
    reg_cmd("kill", h);
}


void process_group::reg_launch(process_processor & proc_proc) {

    po::options_description opt_desc("Command options");
    opt_desc.add_options()
            ("work-dir", po::value<fs::path>(), "Working directory")
            ("save-default", "Save launch options as default");


    auto func = [&proc_proc, opt_desc](const auto & cmd, const auto & chandler) {
        // parsing --args
        std::vector<std::string> opts;
        std::vector<std::string> arg_opts;
        bool args_found = util::parse_args_command_line(cmd.begin(),
                                                        cmd.end(),
                                                        opts,
                                                        arg_opts);

        // parsing options
        po::variables_map var_map;
        po::store(po::command_line_parser{opts}.options(opt_desc).run(), var_map);

        // building launch options

        fs::path work_dir;

        if (var_map.count("work-dir") > 0) {
            work_dir = var_map["work-dir"].as<fs::path>();
        }

        bool save = var_map.count("save-default") > 0;

        // launching target
        proc_proc.launch(work_dir, arg_opts, save, [chandler](const auto & res) {
            chandler(make_result_msg("launched process", res));
        });
    };

    auto handler = function_command::make(func);
    handler->set_desc("Launch process");

    std::ostringstream help_msg;
    help_msg << "Launch process\n\n";
    help_msg << "Command syntax: process launch [options] [--args args]\n";
    help_msg << opt_desc << "\n";
    handler->set_help_msg(help_msg.str());

    reg_cmd("launch", handler);
}


std::string process_group::make_result_msg(const std::string & msg, const async::result<unsigned long> & res) {
    if (!res.is_ok())
        return res.error();

    std::ostringstream str;
    str << msg;
    if (res.value() != 0) {
        str << " " << res.value();
    }
    return str.str();
}


}
