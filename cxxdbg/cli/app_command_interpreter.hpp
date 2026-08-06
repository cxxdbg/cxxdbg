// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file app_command_interpreter.hpp
/// Contains definition of the app_command_interpreter class.

#pragma once

#include "command_interpreter.hpp"
#include "forward.hpp"


namespace cxxdbg::dbg::cli {


class code_break_processor;
class frame_processor;
class thread_processor;
class watch_processor;
class platform_processor;


/// \class app_command_interpreter
/// Command interpreter for cxxdbg application
class app_command_interpreter {
public:
    /// Type of command result handler
    typedef std::function<void (const std::string &)> result_handler;

    /// Constructor, makes command interpreter with specified references
    /// to all required processor objects
    app_command_interpreter(code_break_processor & break_proc,
                            exec_processor & exec_proc,
                            frame_processor & frame_proc,
                            process_processor & proc_proc,
                            target_processor & target_proc,
                            thread_processor & thread_proc,
                            watch_processor & watch_proc,
                            platform_processor & platform_proc);

    /// Constructor, makes command interpreter with specified reference
    /// to abstract command interpreter processor
    app_command_interpreter(app_processor & proc);

    /// Executes command
    void exec(const std::string & cmd, const result_handler & handl);

    /// Returns list of available commands starting with specified prefix
    std::list<std::string> available_commands(const std::string & prefix);

private:
    /// Registers exec command
    void reg_exec_cmd(exec_processor & proc,
                      const std::string & name,
                      const std::string & dsc);

    command_interpreter interp_;            ///< Generic command interpreter object
};


}


