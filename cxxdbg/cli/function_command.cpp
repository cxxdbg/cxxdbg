// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file function_command.cpp
/// Contains implementation of the function_command class.

#include "function_command.hpp"
#include <sstream>


namespace cxxdbg::dbg::cli {


function_command::function_command(const std::string & dsc,
                                   const std::string & helpmsg,
                                   const handler_function & func):
command{dsc},
func_{func},
help_msg_{helpmsg} {
}


function_command::function_command(const handler_function & func):
func_{func} {
}


void function_command::set_help_msg(const std::string & msg) {
    help_msg_ = msg;
}


void function_command::set_desc_help(const std::string & s) {
    set_desc(s);
    set_help_msg(s);
}


void function_command::exec(const string_vector &cmd,
                                    const completion_handler & chandler) const {
    func_(cmd, chandler);
}


/// Returns help messge for command specified as vector of arguments
void function_command::help(const string_vector & cmd,
                                    const completion_handler & handler) const {

    // command arguments should be empty for displaying simple help message
    if (cmd.empty()) {
        handler(help_msg_);
        return;
    }

    // throwing error if cmd args are not empty
    std::ostringstream msg;
    msg << "command '";

    bool first = true;
    for (auto arg : cmd) {
        if (!first)
            msg << " ";
        else
            first = false;

        msg << arg;
    }

    msg << "' not found";

    throw std::runtime_error(msg.str());
}


function_command_sp function_command::make(const std::string & dsc,
                                                           const std::string & helpmsg,
                                                           const handler_function & func) {

    function_command_sp handl(new function_command(dsc, helpmsg, func));
    return handl;
}


function_command_sp function_command::make(const handler_function & func) {
    return function_command_sp{new function_command{func}};
}


}
