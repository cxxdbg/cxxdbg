// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file app_command_interpreter.cpp
/// Contains implementation of the app_command_interpreter class.

#include "app_command_interpreter.hpp"
#include "app_processor.hpp"
#include "break_group.hpp"
#include "exec_command.hpp"
#include "frame_group.hpp"
#include "platform_group.hpp"
#include "process_group.hpp"
#include "target_group.hpp"
#include "thread_group.hpp"
#include "watch_group.hpp"


namespace cxxdbg::dbg::cli {


app_command_interpreter::app_command_interpreter(code_break_processor & break_proc,
                                                 exec_processor & exec_proc,
                                                 frame_processor &frame_proc,
                                                 process_processor & proc_proc,
                                                 target_processor & targ_proc,
                                                 thread_processor & thread_proc,
                                                 watch_processor & watch_proc,
                                                 platform_processor & platform_proc) {

    // breakpoint group
    command_sp break_grp{new break_group{exec_proc, break_proc}};
    interp_.reg_cmd("breakpoint", break_grp);

    // frame group
    command_sp frame_grp{new frame_group{exec_proc, frame_proc}};
    interp_.reg_cmd("frame", frame_grp);

    // process command
    command_sp proc_grp{new process_group{exec_proc, proc_proc}};
    interp_.reg_cmd("process", proc_grp);

    // target command
    command_sp targ_grp{new target_group{targ_proc, exec_proc}};
    interp_.reg_cmd("target", targ_grp);

    // thread command
    command_sp thread_grp{new thread_group{exec_proc, thread_proc}};
    interp_.reg_cmd("thread", thread_grp);

    // watchpoint command
    interp_.reg_cmd("watchpoint", std::make_shared<watch_group>(exec_proc, watch_proc));

    // platform command
    interp_.reg_cmd("platform", std::make_shared<platform_group>(exec_proc, platform_proc));

    // registering app->exec commands
    reg_exec_cmd(exec_proc, "disassemble", "Disassemble bytes in the executable program");
    reg_exec_cmd(exec_proc, "expression", "Evaluate an expression in the current program context");
    reg_exec_cmd(exec_proc, "memory", "A set of commands for operating on memory");
    reg_exec_cmd(exec_proc, "plugin", "A set of commands for managing or customizing plugin commands");
    reg_exec_cmd(exec_proc, "register", "A set of commands to access thread registers");
    reg_exec_cmd(exec_proc, "settings", "A set of commands for manipulating internal settable debugger variables");
    reg_exec_cmd(exec_proc, "script", "Invoke the script interpreter with provided code and display any results.");
    reg_exec_cmd(exec_proc, "source", "A set of commands for accessing source file information");
    reg_exec_cmd(exec_proc, "type", "A set of commands for operating on the type system");
    reg_exec_cmd(exec_proc, "log", "A set of commands for log settings");

    // registering aliases which should implemented as commands
    reg_exec_cmd(exec_proc, "l", "Implements the GDB 'list' command");
    reg_exec_cmd(exec_proc, "list", "Implements the GDB 'list' command");

    // registering aliases
    interp_.reg_alias("attach",     "process attach");
    interp_.reg_alias("add-dsym",   "target symbols add");
    interp_.reg_alias("b",          "breakpoint set");
    interp_.reg_alias("bt",         "thread backtrace");
    interp_.reg_alias("c",          "process continue");
    interp_.reg_alias("call",       "expression --");
    interp_.reg_alias("continue",   "process continue");
    interp_.reg_alias("d",          "breakpoint delete");
    interp_.reg_alias("detach",     "process detach");
    interp_.reg_alias("di",         "disassemble");
    interp_.reg_alias("dis",        "disassemble");
    interp_.reg_alias("expr",       "expression");
    interp_.reg_alias("e",          "expression");
    interp_.reg_alias("f",          "frame select");
    interp_.reg_alias("file",       "target create");
    interp_.reg_alias("finish",     "thread step-out");
    interp_.reg_alias("image",      "target modules");
    interp_.reg_alias("kill",       "process kill");
    interp_.reg_alias("n",          "thread step-over");
    interp_.reg_alias("next",       "thread step-over");
    interp_.reg_alias("nexti",      "thread step-inst-over");
    interp_.reg_alias("ni",         "thread step-inst-over");
    interp_.reg_alias("p",          "expression --");
    interp_.reg_alias("po",         "expression -O  --");
    interp_.reg_alias("print",      "expression --");
    interp_.reg_alias("r",          "process launch");
    interp_.reg_alias("run",        "process launch");
    interp_.reg_alias("s",          "thread step-in");
    interp_.reg_alias("si",         "thread step-inst");
    interp_.reg_alias("step",       "thread step-in");
    interp_.reg_alias("stepi",      "thread step-inst");
    interp_.reg_alias("t",          "thread select");
    interp_.reg_alias("x",          "memory read");
}


app_command_interpreter::app_command_interpreter(app_processor & proc):
app_command_interpreter{proc, proc, proc, proc, proc, proc, proc, proc} {
}


void app_command_interpreter::exec(const std::string & cmd,
                                   const result_handler & handl) {
    interp_.exec(cmd, handl);
}


void app_command_interpreter::reg_exec_cmd(exec_processor & proc,
                                           const std::string & name,
                                           const std::string & dsc) {

    auto handler = exec_command::make(proc, name, dsc);
    interp_.reg_cmd(name, handler);
}

std::list<std::string> app_command_interpreter::available_commands(const std::string & prefix) {
    return interp_.commands_filtered(prefix);
}


}
