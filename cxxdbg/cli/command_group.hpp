// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file command_group.hpp
/// Contains definition of the command_group class.

#pragma once

#include "command.hpp"
#include "forward.hpp"
#include "function_command.hpp"

#include <map>
#include <list>


namespace cxxdbg::dbg::cli {


/// \class group_command_handler
/// Group of commands
class command_group: public command {
public:
    /// Constructor, makes group command handler with specified
    /// brief group description
    command_group(const std::string & dsc);

    /// Destructor, destroys object
    virtual ~command_group();

    /// Executes command represented as vector of arguments and calls
    /// completion handler on completion
    void exec(const string_vector &cmd, const completion_handler & chandler) const override;

    /// Returns help messge for command specified as vector of arguments
    void help(const string_vector & cmd, const completion_handler & handler) const override;

    /// Registers subcommand in group
    void reg_cmd(const std::string & name, const command_sp & handl);

    /// Registers function subcommand
    void reg_func_cmd(const std::string & name,
                      const std::string & dsc,
                      const std::string & helpmsg,
                      const function_command::handler_function & f);

    /// Returns brief description of group or subcommand
    const std::string & desc(const string_vector & cmd) const override;

    /// Returns list of command names which start from selected prefix
    std::list<std::string> commands_filtered(const std::string & prefix);

private:
    /// Type of map from string to command pointer
    typedef std::map<std::string, command_sp> commands_map;


    commands_map subcommands_;          ///< Map of subcommands
};


}


