// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file command_interpreter.cpp
/// Contains implementation of the command_interpreter class.

#include "command_interpreter.hpp"
#include "exec_command.hpp"
#include "cxxdbg/util/command_line_parser.hpp"
#include <algorithm>
#include <sstream>


namespace cxxdbg::dbg::cli {


command_interpreter::command_interpreter():
root_grp_{"root"} {
}


void command_interpreter::exec(const std::string & cmd, const result_handler & handl) {

    // splitting command into arguments
    std::istringstream str(cmd);
    std::vector<std::string> cmd_args;
    util::split_command_line(str, cmd_args);

    if (cmd_args.empty()) {
        handl("\n");
        return;
    }

    // checking for help command
    bool display_help = false;
    if (cmd_args.front() == "help") {
        display_help = true;
        std::vector<std::string> help_args{cmd_args.begin() + 1, cmd_args.end()};
        cmd_args = help_args;
    }

    // looking for alias
    {
        auto it = aliases_.find(cmd_args.front());
        if (it != aliases_.end()) {

            std::vector<std::string> aliased_cmd_args;
            std::copy(it->second.begin(),
                      it->second.end(),
                      std::back_inserter(aliased_cmd_args));
            std::copy(cmd_args.begin() + 1,
                      cmd_args.end(),
                      std::back_inserter(aliased_cmd_args));

            cmd_args = aliased_cmd_args;
        }
    }


    try {
        if (display_help) {
            // displaying help message
            root_grp_.help(cmd_args, [this, handl, cmd_args](const auto & cmdhelp) {

                if (!cmd_args.empty()) {
                    // displaying help for single command or alias
                    handl(cmdhelp);
                    return;
                }


                // displaying help for interpreter, should add aliases

                std::ostringstream resmsg;
                resmsg << cmdhelp;

                resmsg << "\nCommand aliases:\n\n";

                // calculating size of alias name column
                auto comp_alias = [](const auto & c1, const auto & c2) {
                    return c1.first.size() < c2.first.size();
                };
                auto max_sized_alias = std::max_element(aliases_.begin(),
                                                        aliases_.end(),
                                                        comp_alias);
                std::size_t max_alias_size = max_sized_alias->first.size();

                for (const auto & alias : aliases_) {
                    resmsg << "    " << alias.first;

                    std::size_t nspaces = max_alias_size - alias.first.size();
                    for (std::size_t i = 0; i < nspaces; ++i)
                        resmsg << " ";

                    resmsg << " -- " << root_grp_.desc(alias.second) << "\n";
                }

                handl(resmsg.str());
            });
        } else {
            // executing command in root group
            root_grp_.exec(cmd_args, handl);
        }
    }
    catch (std::exception & ex) {
        handl(std::string("error: ") + ex.what());
    }
}


void command_interpreter::reg_cmd(const std::string & name, const command_sp & handl) {
    root_grp_.reg_cmd(name, handl);
}


void command_interpreter::reg_alias(const std::string & name, const std::string & cmd) {
    // parsing alias command
    std::vector<std::string> args;
    std::istringstream str(cmd);
    util::split_command_line(str, args);

    // adding alias
    aliases_.insert(std::make_pair(name, args));
}

std::list<std::string> command_interpreter::commands_filtered(const std::string & prefix) {
    return root_grp_.commands_filtered(prefix);
}


}
