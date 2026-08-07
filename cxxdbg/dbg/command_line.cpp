// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file command_line.cpp
/// Contains implementation of command_line class.

#include "command_line.hpp"
#include "cxxdbg/app/version.hpp"
#include "cxxdbg/log/log_init.hpp"
#include "cxxdbg/util/command_line_parser.hpp"
#include <ranges.hpp>


namespace po = boost::program_options;


namespace cxxdbg::dbg {


command_line::command_line():
opt_desc_("Command line options"),
help_(false) {

    // initializing options description
    opt_desc_.add_options()
            ("help,h", "Display help message and exit")
            ("version,v", "Display version information and exit")
            ("platform", po::value<std::string>()->default_value({}), "Platform name to connect")
            ("platform-url", po::value<std::string>()->default_value({}), "Platform URL to connect")
            ("exec", po::value<std::string>(), "Executable file")
            ("init", po::value<std::string>()->default_value({}), "Semicolon separated list of initialization commands")
            ("exec-search-paths", po::value<std::string>()->default_value({}), "Semicolon separated list of module search paths");

    opt_desc_.add(cxxdbg::log::log_options());

    // initializing position options description
    pos_opt_desc_.add("exec",1);
}


void command_line::parse(int argc, char * argv[]) {

    // removing options after --args

    std::vector<std::string> opts;
    exe_args_.clear();

    bool args_parsed = util::parse_args_command_line(argv + 1,
                                                     argv + argc,
                                                     opts,
                                                     exe_args_);


    // parsing option with boost::program_options

    po::store(po::command_line_parser(opts).options(opt_desc_).positional(pos_opt_desc_).run(), vars_);

    help_ = vars_.count("help") > 0;
    version_ = vars_.count("version") > 0;

    platform_name_ = vars_["platform"].as<std::string>();
    platform_url_ = vars_["platform-url"].as<std::string>();

    {
        auto init = vars_["init"].as<std::string>();
        auto cmds_ranges = init | std::ranges::views::split(';');
        for (auto && cmd : cmds_ranges) {
            auto cmd_comm = cmd | std::ranges::views::common;
            init_cmds_.push_back(std::string{std::ranges::begin(cmd_comm), std::ranges::end(cmd_comm)});
        }
    }

    {
        auto paths = vars_["exec-search-paths"].as<std::string>();
        auto paths_ranges = paths | std::ranges::views::split(';');
        for (auto && p : paths_ranges) {
            auto p_comm = p | std::ranges::views::common;
            exec_search_paths_.push_back(std::string{std::ranges::begin(p_comm), std::ranges::end(p_comm)});
        }
    }

    // process --exec option
    if (vars_.count("exec") > 0) {
        if (args_parsed) {
            throw std::runtime_error("--exec and --args both specified");
        }

        exe_name_ = vars_["exec"].as<std::string>();
    } else if (args_parsed) {
        if (exe_args_.empty()) {
            throw std::runtime_error("--args specified but no executable specified");
        }

        exe_name_ = exe_args_.front();
        exe_args_.erase(exe_args_.begin());
    }
}


void command_line::print_help(std::ostream & str, std::string_view product_name) const {
    std::string ver_str = std::string{product_name} + " version " + cxxdbg::version_display_str();
    str << ver_str << "\n\n"
        << "Usage: cxxdbg [options] [--args|-- [debuggee command line]]\n\n"
        << opt_desc_;
}


void command_line::print_version(std::ostream & str, std::string_view product_name) const {
    std::string ver_str = std::string{product_name} + " version " + cxxdbg::version_display_str();
    str << ver_str << "\n";
}


}
