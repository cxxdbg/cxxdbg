// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file exec_processor.hpp
/// Contains definition of the exec_processor class.

#pragma once

#include <functional>
#include <string>


namespace cxxdbg::dbg::cli {


/// Abstract command processor which is used in by exec_command
/// to redirect command execution
class exec_processor {
public:
    /// Type of result handler function
    typedef std::function<void (const std::string &)> result_handler;

    /// Destructor, destroys object
    virtual ~exec_processor() {}

    /// Process command with specified command string and handler
    virtual void exec_cmd(const std::string & cmd, const result_handler & handl) = 0;
};


}


