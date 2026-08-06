// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file target_group.hpp
/// Contains definition of the target_group class.

#pragma once

#include "command_group.hpp"
#include "forward.hpp"


namespace cxxdbg::dbg::cli {


/// Target group commands
class target_group: public command_group {
public:
    /// Constructor, makes target command handler with specifeid reference
    /// to abstract target command processor, and reference to app->exec
    /// command processor
    target_group(target_processor & targ_proc, exec_processor & app_proc);

    /// Destructor, destroys object
    virtual ~target_group();

private:
    /// Registers app exec command
    void reg_app_exec_cmd(const std::string & name, const std::string & desc);

    target_processor & targ_proc_;          ///< Reference to target command processor
    exec_processor & app_proc_;///< Reference to app exec command processor
};


}


