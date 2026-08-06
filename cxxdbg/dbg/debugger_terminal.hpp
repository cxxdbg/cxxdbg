// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file debugger_terminal.hpp
/// Contains definition of the debugger_terminal class.

#pragma once

#include "cxxdbg/app/signals.hpp"
#include <functional>
#include <list>
#include <string>


namespace cxxdbg {

class settings_storage;

namespace dbg {

namespace cli {
    class app_command_interpreter;
}


/// Represents debugger terminal. Contains logic for saving commands
/// history
class debugger_terminal {
public:
    /// Type of exec result handler
    typedef std::function<void (std::string)> result_handler;

    /// Default history size
    static const unsigned int def_hist_size = 100;

    /// Constructor, makes debugger terminal object with specified reference
    /// to setting storage, reference to command interpreter, and history size
    debugger_terminal(settings_storage & sett,
                      cli::app_command_interpreter & interp,
                      unsigned int hsize = def_hist_size);

    /// Destructor, destroys object
    ~debugger_terminal();

    /// Executes command
    void exec(const std::string & cmd, const result_handler & h = {});

    /// Sets current command to previous. Returns previous command line
    /// or empty string if there is no previous command
    std::string prev_cmd();

    /// Sets current command to next. Return next command line or
    /// empty string if there is no next command
    std::string next_cmd();

    /// The signal is emitted when terminal output is received
    signal<void (std::string)> output_received;

private:
    settings_storage & sett_;               ///< Reference to settings storage
    cli::app_command_interpreter & interp_; ///< Reference to command interpreter
    unsigned int hist_size_;                ///< History size
    std::list<std::string> hist_;           ///< Command history

    ///< Iterator pointing to current command in history, or
    /// to end(hist_) if no current command
    std::list<std::string>::const_iterator cmd_it_;
};


} }


