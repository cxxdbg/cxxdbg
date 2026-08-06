// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file exec_command.hpp
/// Contains definition of the exec_command class.

#pragma once

#include "command.hpp"
#include "forward.hpp"


namespace cxxdbg::dbg::cli {


/// Command implementation which redirects command execution
/// to abstract exec processor object passed in constructor
class exec_command: public command {
public:
    /// Constructor, constructs command with specified
    /// reference to exec processor, prefix string which
    /// is passed to processor before command arguments, and
    /// command brief description
    exec_command(exec_processor & proc,
                 const std::string & prefix,
                 const std::string & dsc);

    /// Destructor, destroys object
    virtual ~exec_command();

    /// Executes command represented as vector of arguments and calls
    /// completion handler on completion
    virtual void exec(const string_vector & cmd,
                      const completion_handler & chandler) const;

    /// Returns help messge for command specified as vector of arguments
    virtual void help(const string_vector & cmd,
                      const completion_handler & handler) const;

    /// Makes command handler and wraps it in shared pointer
    static exec_command_sp make(
            exec_processor & proc,
            const std::string & prefix,
            const std::string & dsc);

private:
    exec_processor & proc_;///< Reference to command processor
    std::string prefix_;                        ///< Prefix string
};


}


