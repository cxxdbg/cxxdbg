// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file break_group.cpp
/// Contains implementation of the break_group class.

#include "break_group.hpp"
#include "code_break_processor.hpp"
#include "exec_command.hpp"
#include "po_command.hpp"
#include "cxxdbg/util/print.hpp"


namespace fs = std::filesystem;
namespace po = boost::program_options;


namespace cxxdbg::dbg::cli {


break_group::break_group(exec_processor & exec_proc, code_break_processor & break_proc):
break_base_group{breakpoint_type::code, "breakpoint", exec_proc, break_proc} {
    reg_set(break_proc);
}


void break_group::reg_set(code_break_processor & proc) {
    auto cmd = make_po_command([&proc](const po::variables_map & vars, auto && h) {

        // generic completion handler for adding breakpoint
        auto add_handler = [h](auto && res) {
            if (res.is_ok()) {
                std::ostringstream msg;
                msg << "added breakpoint " << res.value();
                h(msg.str());
            } else {
                h(res.error());
            }
        };

        // processing base breakpoint options
        break_properties props;
        process_break_options(vars, props);

        bool has_common_options = vars.count("location") > 0 || vars.count("function") > 0 ||
                vars.count("address") > 0 || vars.count("file") > 0 || vars.count("line") > 0;

        bool has_exception_thrown = vars.count("exception-thrown") > 0;
        bool has_exception_caught = vars.count("exception-caught") > 0;
        if (has_exception_thrown || has_exception_caught) {
            if (has_common_options) {
                h("cannot combine exception-thrown or exception-caught options with location, function, address, file or line options");
                return;
            }

            if (has_exception_thrown) {
                proc.add_exception_thrown_breakpoint(props, add_handler);
            }

            if (has_exception_caught) {
                proc.add_exception_caught_breakpoint(props, add_handler);
            }

            return;
        }

        if (vars.count("location") > 0) {

            // location expression

            std::string expr = vars["location"].as<std::string>();

            if (expr.empty()) {
                h("location expression is empty");
                return;
            }

            // checking for address expression
            if (expr[0] == '*') {
                // address

                std::string saddr = expr.substr(1);
                std::uint64_t addr;
                bool res = util::parse_hex_uint(saddr, addr);
                if (!res) {
                    h("location address is invalid");
                    return;
                }

                proc.add_address_breakpoint(addr, props, add_handler);
                return;
            }

            // checking for file:line
            std::size_t pos = expr.find(':');
            if (pos != std::string::npos && (pos == expr.size() || expr[pos + 1] != ':')) {
                // file:line

                std::string file = expr.substr(0, pos);
                std::string sline = expr.substr(pos + 1, expr.size() - pos - 1);
                unsigned int line;
                bool res = util::parse_hex_uint(sline, line);
                if (!res) {
                    h(std::string{"line number in location is invalid: '"} + sline + "'");
                    return;
                }

                proc.add_srcpos_breakpoint(file, line, props, add_handler);
                return;
            }

            // function
            proc.add_function_breakpoint(expr, props, add_handler);

        } else if (vars.count("file") > 0 || vars.count("line") > 0) {

            // source position breakpoint

            if (vars.count("line") == 0) {
                h("line number is not specified");
                return;
            }
            if (vars.count("file") == 0) {
                h("file name is not specified");
                return;
            }
            if (vars.count("function") > 0) {
                h("both file and function specified");
                return;
            }
            if (vars.count("address") > 0) {
                h("both file and address specified");
                return;
            }

            fs::path file = vars["file"].as<fs::path>();
            unsigned int line = vars["line"].as<unsigned int>();

            proc.add_srcpos_breakpoint(file, line, props, add_handler);

        } else if (vars.count("function") > 0) {

            // function breakpoint

            if (vars.count("address") > 0) {
                h("both function and address specified");
                return;
            }

            std::string func = vars["function"].as<std::string>();
            proc.add_function_breakpoint(func, props, add_handler);

        } else if (vars.count("address") > 0) {

            // address breakpoint

            std::string saddr = vars["address"].as<std::string>();
            std::uint64_t addr;
            bool res = util::parse_hex_uint(saddr, addr);
            if (!res) {
                h("invalid address value");
                return;
            }

            proc.add_address_breakpoint(addr, props, add_handler);
        }
    });

    cmd->set_desc_help("Set breakpoint");
    cmd->opt_desc().add_options()
            ("exception-thrown", "Set breakpoint when exception thrown")
            ("exception-caught", "Set breakpoint when exception caught")
            ("location", po::value<std::string>(), "Set breakpoint at location specified by GDB syntax")
            ("address,a", po::value<std::string>(), "Set breakpoint at memory address")
            ("function,b", po::value<std::string>(), "Set breakpoint at function")
            ("file,f", po::value<fs::path>(), "Set breakpoint at source position in file")
            ("line,l", po::value<unsigned int>(), "Set breakpoint at line");
    add_break_options("Breakpoint", cmd->opt_desc());
    cmd->popt_desc().add("location", 1);
    reg_cmd("set", cmd);
}


}
