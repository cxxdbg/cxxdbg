// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file command_group.cpp
/// Contains implementation of the command_group class.

#include "command_group.hpp"
#include <algorithm>
#include <cassert>
#include <iomanip>
#include <sstream>


namespace cxxdbg::dbg::cli {


command_group::command_group(const std::string & dsc):
command{dsc} {
}


command_group::~command_group() {
}


void command_group::exec(const string_vector &cmd,
                         const completion_handler & chandler) const {

    if (cmd.empty()) {
        // display help if command is not specified
        help({}, chandler);
        return;
    }

    const std::string & cmd_name = cmd.front();

    // searching for subcommand in command map
    auto it = subcommands_.find(cmd_name);
    if (it == subcommands_.end()) {
        std::ostringstream str;
        str << "command '" << cmd_name << "' not found";
        throw std::runtime_error(str.str());
    }

    // executing command
    try {
        string_vector cmd_args(cmd.begin() + 1, cmd.end());
        it->second->exec(cmd_args, chandler);
    }
    catch (std::exception & ex) {
        std::ostringstream msg;
        msg << cmd_name << ": " << ex.what();
        throw std::runtime_error(msg.str());
    }
}


std::size_t do_transform(const std::pair<const std::string, command_sp> & p) {
    return p.first.size();
}


void command_group::help(const string_vector & cmd,
                                 const completion_handler & handler) const {
    if (cmd.empty()) {
        // showing help for group

        // calculating size of command name column
        auto comp_cmd = [](const auto & c1, const auto & c2) {
            return c1.first.size() < c2.first.size();
        };
        auto max_sized_cmd = std::max_element(subcommands_.begin(),
                                              subcommands_.end(),
                                              comp_cmd);
        std::size_t max_cmd_size = max_sized_cmd->first.size();

        // showing help
        std::ostringstream msg;
        msg << "Supported commands:\n\n";
        for (auto scmd : subcommands_) {

            msg << "    " << scmd.first;

            std::size_t nspaces = max_cmd_size - scmd.first.size();
            for (std::size_t i = 0; i < nspaces; ++i)
                msg << " ";

            msg << " -- " << scmd.second->desc({}) << "\n";
        }

        handler(msg.str());

    } else {
        // showing help for command

        const std::string & cmd_name = cmd.front();

        auto it = subcommands_.find(cmd_name);
        if (it == subcommands_.end()) {
            std::ostringstream str;
            str << "command '" << cmd_name << "' not found";
            throw std::runtime_error(str.str());
        }

        try {
            string_vector cmd_args(cmd.begin() + 1, cmd.end());
            it->second->help(cmd_args, handler);
        }
        catch (std::exception & ex) {
            std::ostringstream msg;
            msg << cmd_name << ": " << ex.what();
            throw std::runtime_error(msg.str());
        }
    }
}


void command_group::reg_cmd(const std::string & name, const command_sp & handl) {

    // checking that command has description
    assert(!handl->desc({}).empty() && "Command has empty description");

    auto res = subcommands_.insert(std::make_pair(name, handl));
    assert(res.second && "Subcommand with specified name already exists");
}


void command_group::reg_func_cmd(const std::string & name,
                                         const std::string & dsc,
                                         const std::string & helpmsg,
                                         const function_command::handler_function & f) {

    command_sp handl(new function_command(dsc, helpmsg, f));
    reg_cmd(name, handl);
}


const std::string & command_group::desc(const string_vector & cmd) const {
    if (cmd.empty()) {
        return command::desc({});
    }

    // trying find subcommand
    auto it = subcommands_.find(cmd.front());
    if (it == subcommands_.end()) {
        // command not found
        return command::desc({});
    }

    string_vector new_cmd{cmd.begin() + 1, cmd.end()};
    return it->second->desc(new_cmd);
}

namespace {
bool starts_with(const std::string & str, const std::string & prefix) {
    return str.substr(0, prefix.length()) == prefix;
}
}

std::list<std::string> command_group::commands_filtered(const std::string & prefix) {

    std::list<std::string> filtered_commands;
    for (auto it : subcommands_) {
        auto name = it.first;
        if (starts_with(name, prefix))
            filtered_commands.push_back(name);
    }

    return filtered_commands;
}

}
