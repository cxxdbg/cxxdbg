// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file function_command.hpp
/// Contains definition of the function_command class.

#pragma once

#include "command.hpp"
#include "forward.hpp"


namespace cxxdbg::dbg::cli {


/// \class function_command
/// Custom command which processes arguments using specified std::function
class function_command: public command {
public:
    /// Type of handler function
    typedef std::function<void (const string_vector &,
                                const completion_handler &)> handler_function;

    /// Constructor, makes command handler with specifeid brief command description,
    /// help message, and function for handling commands
    function_command(const std::string & dsc,
                     const std::string & hlpmsg,
                     const handler_function & func);

    /// Constructor, makes command handler with specifeid function object
    function_command(const handler_function & func);

    /// Sets help message
    void set_help_msg(const std::string & msg);

    /// Sets description and help message
    void set_desc_help(const std::string & s);

    /// Executes command represented as vector of arguments and calls
    /// completion handler on completion
    virtual void exec(const string_vector & cmd,
                      const completion_handler & chandler) const;

    /// Returns help messge for command specified as vector of arguments
    virtual void help(const string_vector & cmd,
                      const completion_handler & handler) const;

    /// Makes new function command handler and wraps it into shared pointer
    static function_command_sp make(const std::string & dsc,
                                            const std::string & helpmsg,
                                            const handler_function & func);

    /// Makes new function command handler and wraps it into shared pointer
    static function_command_sp make(const handler_function & func);

private:
    handler_function func_;         ///< Handler function
    std::string help_msg_;          ///< Help message
};


}


