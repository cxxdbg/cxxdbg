// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file exec_command.cpp
/// Contains implementation of the exec_command class.

#include "exec_command.hpp"
#include "exec_processor.hpp"
#include "cxxdbg/util/command_line_parser.hpp"
#include <sstream>


namespace cxxdbg::dbg::cli {


exec_command::exec_command(exec_processor & proc,
                           const std::string & prefix,
                           const std::string & dsc):
command{dsc},
proc_{proc},
prefix_{prefix} {
}


exec_command::~exec_command() {
}


void exec_command::exec(const string_vector & cmd,
                        const completion_handler & chandler) const {
    // building command line
    std::ostringstream cmdline;
    cmdline << prefix_;

    if (!cmd.empty()) {
        cmdline << " ";
        util::write_command_line(cmdline, cmd);
    }

    // executing command in processor
    proc_.exec_cmd(cmdline.str(), chandler);
}


void exec_command::help(const string_vector & cmd,
                        const completion_handler & handler) const {
    // building command line
    std::ostringstream cmdline;
    cmdline << "help " << prefix_;

    if (!cmd.empty()) {
        cmdline << " ";
        util::write_command_line(cmdline, cmd);
    }

    // executing command in processor
    proc_.exec_cmd(cmdline.str(), handler);
}


exec_command_sp exec_command::make(
        exec_processor & proc,
        const std::string & prefix,
        const std::string & dsc) {

    return exec_command_sp(new exec_command(proc, prefix, dsc));
}


}
