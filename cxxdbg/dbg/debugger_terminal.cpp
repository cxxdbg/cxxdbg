// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file debugger_terminal.cpp
/// Contains implementation of the debugger_terminal class.

#include "debugger_terminal.hpp"
#include "cxxdbg/app/settings_storage.hpp"
#include "cxxdbg/cli/app_command_interpreter.hpp"


namespace cxxdbg::dbg {


const unsigned int debugger_terminal::def_hist_size;


debugger_terminal::debugger_terminal(settings_storage & sett,
                                     cli::app_command_interpreter & interp,
                                     unsigned int hsize):
sett_{sett},
interp_{interp},
hist_size_{hsize} {
    // reading history from settings
    hist_ = sett_.read<std::list<std::string>>("debugger_terminal/history", {});

    // trimming history
    while (hist_.size() > hist_size_)
        hist_.pop_front();

    cmd_it_ = hist_.end();
}


debugger_terminal::~debugger_terminal() {
}


void debugger_terminal::exec(const std::string & cmd, const result_handler & h) {
    // adding command to history
    hist_.push_back(cmd);
    if (hist_.size() > hist_size_)
        hist_.pop_front();
    cmd_it_ = hist_.end();

    // saving history to settings
    sett_.write("debugger_terminal/history", hist_);

    // sending command to output
    std::string cmd_output = "> ";
    cmd_output += cmd;
    output_received(cmd_output);

    // passing command to intepreter
    interp_.exec(cmd, [this, h](auto && res) {
        // notifying clients about output
        output_received(res);

        // executing handler
        if (h) {
            h(res);
        }
    });
}


std::string debugger_terminal::prev_cmd() {
    if (cmd_it_ == std::begin(hist_))
        return "";

    --cmd_it_;
    return *cmd_it_;
}


std::string debugger_terminal::next_cmd() {
    if (cmd_it_ == std::end(hist_))
        return "";

    ++cmd_it_;
    if (cmd_it_ == std::end(hist_))
        return "";

    return *cmd_it_;
}


}
