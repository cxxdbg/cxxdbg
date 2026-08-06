// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file command.hpp
/// Contains definition of the command class.

#pragma once

#include <functional>
#include <string>
#include <vector>


namespace cxxdbg::dbg::cli {


/// Represents abstract command
class command {
public:
    /// Type of vector of strings
    typedef std::vector<std::string> string_vector;

    /// Type of completion handler
    typedef std::function<void (const std::string &)> completion_handler;

    /// Constructor, makes command handler with specified brief description
    command(const std::string & dsc = std::string());

    /// Destructor, destroys objectg
    virtual ~command();

    /// Executes command with specified vector of arguments and calls
    /// completion handler on completion
    virtual void exec(const string_vector & cmd,
                      const completion_handler & chandler) const = 0;

    /// Returns help messge for command specified as vector of arguments
    virtual void help(const string_vector & cmd,
                      const completion_handler & handler) const = 0;

    /// Returns brief description of command for specified arguments
    virtual const std::string & desc(const string_vector & cmd) const;

    /// Sets brief description of command
    void set_desc(const std::string & d);

private:
    std::string desc_;          ///< Brief description of command
};


}


