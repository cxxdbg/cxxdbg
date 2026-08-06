// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file watch_group.cpp
/// Contains implementation of the watch_group class.

#include "watch_group.hpp"
#include "command_group.hpp"


namespace cxxdbg::dbg::cli {


namespace po = boost::program_options;


/// Adds watchpoint create options to options descriptions
static void add_watch_create_options(po::options_description & desc) {
    add_break_options("Watchpoint", desc);
    desc.add_options()
            ("byte-size,s", po::value<size_t>(),
                "Number of bytes to use to watch a region. Values: 1 | 2 | 4 | 8")
            ("watch-type,w", po::value<std::string>(),
                "Specify the type of watching to perform. Values: read | write | read_write");
}


/// Processes watch creation options and sets properties
static bool process_watch_create_options(const po::variables_map & vars,
                                         watch_create_properties & props,
                                         std::string & err) {

    process_break_options(vars, props);

    if (vars.count("byte-size") > 0) {
        auto size = vars["byte-size"].as<size_t>();
        if (size != 1 && size != 2 && size != 4 && size != 8) {
            std::ostringstream msg;
            msg << "invalid watch size: " << size;
            err = msg.str();
            return false;
        }

        props.size = size;
    }

    if (vars.count("watch-type") > 0) {
        const auto & wt = vars["watch-type"].as<std::string>();
        if (wt == "read") {
            props.is_read = true;
            props.is_write = false;
        } else if (wt == "write") {
            props.is_read = false;
            props.is_write = true;
        } else {
            assert(wt == "read_write" && "Unknown watch-type option value");
            props.is_read = true;
            props.is_write = true;
        }
    }

    return true;
}


watch_group::watch_group(exec_processor & exec_proc, watch_processor & watch_proc):
break_base_group{breakpoint_type::watch, "watchpoint", exec_proc, watch_proc} {

    // registering set command group
    auto set_cmd = std::make_shared<command_group>("Sets watchpoint at specified location");
    set_cmd->set_desc("Sets watchpoint at specified location");
    reg_cmd("set", set_cmd);


    // adding variable command

    auto var_cmd = make_po_command([&watch_proc](const po::variables_map & vars, auto && handl) {
        // processing watch creation options
        watch_create_properties props;
        std::string err;
        if (!process_watch_create_options(vars, props, err)) {
            handl(err);
            return;
        }

        // checking that variable option is specified
        if (vars.count("variable") == 0) {
            handl("variable name is not specified");
            return;
        }

        // creating watchpoint
        watch_proc.add_var_watchpoint(vars["variable"].as<std::string>(), props,
        [handl](auto && res) {
            std::ostringstream msg;

            if (res.is_ok()) {
                msg << "added watchpoint " << res.value();
            } else {
                msg << "error creating watchpoint: " << res.error();
            }

            handl(msg.str());
        });
    });

    var_cmd->set_desc("Set a watchpoint on a variable");
    set_cmd->reg_cmd("variable", var_cmd);
    var_cmd->opt_desc().add_options()
            ("variable,v", po::value<std::string>(), "Name of variable");
    add_watch_create_options(var_cmd->opt_desc());
    var_cmd->popt_desc().add("variable", 1);


    // adding expresson command

    auto expr_cmd = make_po_command([&watch_proc](const po::variables_map & vars, auto && handl) {
        // processing watch creation options
        watch_create_properties props;
        std::string err;
        if (!process_watch_create_options(vars, props, err)) {
            handl(err);
            return;
        }

        // checking that expression option is specified
        if (vars.count("expression") == 0) {
            handl("expression value is not specified");
            return;
        }

        // creating watchpoint
        watch_proc.add_expr_watchpoint(vars["expression"].as<std::string>(), props,
        [handl](auto && res) {
            std::ostringstream msg;

            if (res.is_ok()) {
                msg << "added watchpoint " << res.value();
            } else {
                msg << "error creating watchpoint: " << res.error();
            }

            handl(msg.str());
        });
    });

    expr_cmd->set_desc("Set a watchpoint on an address by supplying an expression");
    set_cmd->reg_cmd("expression", expr_cmd);
    expr_cmd->opt_desc().add_options()
            ("expression,e", po::value<std::string>(), "Address expression");
    add_watch_create_options(expr_cmd->opt_desc());
    expr_cmd->popt_desc().add("expression", 1);
}


}
