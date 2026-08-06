// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file platform_group.hpp
/// Contains definition of the platform_group class.

#pragma once

#include "command_group.hpp"
#include "forward.hpp"
#include "platform_processor.hpp"


namespace cxxdbg::dbg::cli {


/// Group of platform commands
class platform_group: public command_group {
public:
    /// Constructor, makes handler with specified references to app-exec and platform processors
    platform_group(exec_processor & exec_proc, platform_processor & p_proc);

private:
    /// Registers exec command that will be passed to exec_processor
    void reg_exec_cmd(exec_processor & app_proc, const std::string & name, const std::string & desc);

    /// Registers select command
    void reg_select(platform_processor & proc);

    /// Registers connect command
    void reg_connect(platform_processor & proc);

    /// Registers disconnect command
    void reg_disconnect(platform_processor & proc);
};


}
