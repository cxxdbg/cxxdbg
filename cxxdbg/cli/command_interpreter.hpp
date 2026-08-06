// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file command_interpreter.hpp
/// Contains definition of the command_interpreter class.

#pragma once

#include "forward.hpp"
#include "command_group.hpp"
#include <functional>
#include <string>


namespace cxxdbg::dbg::cli {


/// \class command_interpreter
/// Represents generic command interpreter for processing commands
class command_interpreter {
public:
    /// Type of command result handler
    typedef std::function<void (const std::string &)> result_handler;

    /// Constructor, makes command interpreter with specified
    /// reference to application object
    command_interpreter();

    /// Executes command
    void exec(const std::string & cmd, const result_handler & handl);

    /// Registers root command with specified name and command handler
    void reg_cmd(const std::string & name, const command_sp & handl);

    /// Registers command alias
    void reg_alias(const std::string & name, const std::string & cmd);

    /// Returns list of commands starting with prefix
    std::list<std::string> commands_filtered(const std::string & prefix);

private:
    /// Type of map of aliases
    typedef std::map<std::string, std::vector<std::string>> alias_map;

    command_group root_grp_;            ///< Root command group
    alias_map aliases_;                         ///< Map of command aliases
};


}


