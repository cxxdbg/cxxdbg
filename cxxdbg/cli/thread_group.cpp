// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread_group.cpp
/// Contains implementation of the thread_group class.

#include "thread_group.hpp"
#include "exec_command.hpp"
#include "po_command.hpp"
#include "thread_processor.hpp"
#include "cxxdbg/async/result.hpp"


namespace po = boost::program_options;


namespace cxxdbg::dbg::cli {


void step_command_handler(const async::result<> & res, const command::completion_handler & h) {
    if (res.is_ok()) {
        h("stepping");
    } else {
        h(res.error());
    }
}


/// Makes stepping program options command with specified action on thread processor
template <typename Func>
static std::shared_ptr<po_command_base> make_step_command(thread_processor & proc, const Func & f) {
    return make_po_command([&proc, f](const auto & vars, const auto & h) {
        f(proc, [h](const auto & res) {
            step_command_handler(res, h);
        });
    });
}


thread_group::thread_group(exec_processor & exec_proc, thread_processor & thread_proc):
command_group{"A set of commands for manipulating threads"} {

    reg_exec(exec_proc, "backtrace",    "Show call stack");
    reg_exec(exec_proc, "info",         "Show information about thread");
    reg_exec(exec_proc, "jump",         "Set program counter to new address");
    reg_exec(exec_proc, "list",         "Show list of threads in process");

    reg_step_in(thread_proc);
    reg_step_over(thread_proc);
    reg_step_out(thread_proc);
    reg_select(thread_proc);
    reg_until(thread_proc);

    auto isic = make_step_command(thread_proc, std::mem_fn(&thread_processor::inst_step_into));
    isic->set_desc_help("Instruction step into");
    reg_cmd("step-inst", isic);

    auto isoc = make_step_command(thread_proc, std::mem_fn(&thread_processor::inst_step_over));
    isoc->set_desc_help("Step over");
    reg_cmd("step-inst-over", isoc);
}


thread_group::~thread_group() {
}


void thread_group::reg_exec(exec_processor & exec_proc,
                            const std::string & name,
                            const std::string & desc) {

    reg_cmd(name, exec_command::make(exec_proc, std::string("thread ") + name, desc));
}


void thread_group::reg_step_in(thread_processor & proc) {
    auto handler = [&proc](const po::variables_map & vars, const auto & h) {
        // checking if both -a -A specified
        if (vars.count("step-in-avoids-no-debug") &&
            vars.count("step-out-avoids-no-debug")) {
            bool in_val = vars["step-in-avoids-no-debug"].as<bool>();
            bool out_val = vars["step-out-avoids-no-debug"].as<bool>();
            if (in_val != out_val) {
                throw std::runtime_error{"different values for -A and -a options not supported"};
            }
        }

        thread_processor::tribool avoid_nodebug = boost::indeterminate;

        // parsing -a option
        if (vars.count("step-in-avoids-no-debug")) {
            if (vars["step-in-avoids-no-debug"].as<bool>()) {
                avoid_nodebug = true;
            } else {
                avoid_nodebug = false;
            }
        }

        // parsing -A option
        if (vars.count("step-out-avoids-no-debug")) {
            if (vars["step-out-avoids-no-debug"].as<bool>()) {
                avoid_nodebug = true;
            } else {
                avoid_nodebug = false;
            }
        }

        // parsing -t option
        std::string target_func;
        if (vars.count("step-in-target")) {
            target_func = vars["step-in-target"].as<std::string>();
        }

        // parsing -r option
        std::string avoid_regex;
        if (vars.count("step-over-regexp")) {
            avoid_regex = vars["step-over-regexp"].as<std::string>();
        }

        // parsing -s option
        std::string step_through_regex;
        if (vars.count("step-through-regexp")) {
            step_through_regex = vars["step-through-regexp"].as<std::string>();
        }

        proc.step_into(avoid_nodebug, target_func, avoid_regex, step_through_regex,
                       [h](const auto & res) {
            step_command_handler(res, h);
        });
    };

    auto cmd = make_po_command(handler);
    cmd->opt_desc().add_options()
            ("step-in-avoids-no-debug,a", po::value<bool>(),
                "A boolean value, if true stepping in/out of functions will continue "
                "to step in or step out untill it hits a function with debug information.")
            ("step-out-avoids-no-debug,A", po::value<bool>(),
                "Same as --step-in-avoids-no-debug")
            ("step-over-regexp,r", po::value<std::string>(),
                "A regular expression that defines function names to not to stop at when stepping in.")
            ("step-through-regexp,s", po::value<std::string>(),
                "A regular expression that defines function names to step through when stepping in.")
            ("step-in-target,t", po::value<std::string>(),
                "The name of the directly called function step in should stop at when stepping into.");


    cmd->set_desc_help("Source level single step in current thread");
    reg_cmd("step-in", cmd);
}


void thread_group::reg_step_over(thread_processor & proc) {
    auto handler = [&proc](const po::variables_map & vars, const auto & h) {
        // parsing -s option
        std::string step_through_regex;
        if (vars.count("step-through-regexp")) {
            step_through_regex = vars["step-through-regexp"].as<std::string>();
        }

        proc.step_over(step_through_regex, [h](const auto & res) {
            step_command_handler(res, h);
        });
    };

    auto cmd = make_po_command(handler);
    cmd->opt_desc().add_options()
            ("step-through-regexp,s", po::value<std::string>(),
                "A regular expression that defines function names to step through when stepping over.");

    cmd->set_desc_help("Source level single step in current thread, stepping over calls");
    reg_cmd("step-over", cmd);
}


void thread_group::reg_step_out(thread_processor & proc) {

    auto handler = [&proc](const po::variables_map & vars, const auto & h) {

        // checking if both -a -A specified
        if (vars.count("step-in-avoids-no-debug") &&
            vars.count("step-out-avoids-no-debug")) {
            bool in_val = vars["step-in-avoids-no-debug"].as<bool>();
            bool out_val = vars["step-out-avoids-no-debug"].as<bool>();
            if (in_val != out_val) {
                throw std::runtime_error{"different values for -A and -a options not supported"};
            }
        }

        thread_processor::tribool avoid_nodebug = boost::indeterminate;

        // parsing -a option
        if (vars.count("step-in-avoids-no-debug")) {
            if (vars["step-in-avoids-no-debug"].as<bool>()) {
                avoid_nodebug = true;
            } else {
                avoid_nodebug = false;
            }
        }

        // parsing -A option
        if (vars.count("step-out-avoids-no-debug")) {
            if (vars["step-out-avoids-no-debug"].as<bool>()) {
                avoid_nodebug = true;
            } else {
                avoid_nodebug = false;
            }
        }

        // parsing -s option
        std::string step_through_regex;
        if (vars.count("step-through-regexp")) {
            step_through_regex = vars["step-through-regexp"].as<std::string>();
        }

        proc.step_out(avoid_nodebug, step_through_regex, [h](const auto & res) {
            step_command_handler(res, h);
        });
    };

    auto cmd = make_po_command(handler);
    cmd->opt_desc().add_options()
            ("step-in-avoids-no-debug,a", po::value<bool>(),
                "A boolean value, if true stepping in/out of functions will continue "
                "to step in or step out untill it hits a function with debug information.")
            ("step-out-avoids-no-debug,A", po::value<bool>(),
                "Same as --step-in-avoids-no-debug")
            ("step-through-regexp,s", po::value<std::string>(),
                "A regular expression that defines function names to step through when stepping out.");

    cmd->set_desc_help("Step out in current thread");
    reg_cmd("step-out", cmd);
}


void thread_group::reg_select(thread_processor & proc) {
    auto cmd = make_po_command([&proc](const po::variables_map & vars, const auto & h) {
        std::size_t index = vars["thread-index"].as<std::size_t>();
        proc.select_thread(index, [index, h](const auto & res) {
            if (res.is_ok()) {
                std::ostringstream msg;
                msg << "selected thread " << index;
                h(msg.str());
            } else {
                h(res.error());
            }
        });
    });

    cmd->opt_desc().add_options()
            ("thread-index", po::value<std::size_t>()->required(), "Thread index");
    cmd->popt_desc().add("thread-index", 1);
    cmd->set_desc_help("Select active thread");
    reg_cmd("select", cmd);
}


void thread_group::reg_until(thread_processor & proc) {
    auto cmd = make_po_command([&proc](const po::variables_map & vars, const auto & h) {
        unsigned int line = vars["line"].as<unsigned int>();
        proc.step_until(line, [line, h](const auto & res) {
            if (res.is_ok()) {
                std::ostringstream msg;
                msg << "stepping to line " << line;
                h(msg.str());
            } else {
                h(res.error());
            }
        });
    });

    cmd->opt_desc().add_options()
            ("line", po::value<unsigned int>()->required(), "Line number");
    cmd->popt_desc().add("line", 1);
    cmd->set_desc_help("Step until line");
    reg_cmd("until", cmd);
}


}
