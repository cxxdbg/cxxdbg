// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file process_group.hpp
/// Contains definition of the process_group class.

#pragma once

#include "command_group.hpp"
#include "forward.hpp"
#include "cxxdbg/async/forward.hpp"


namespace cxxdbg::dbg::cli {


/// Process command group
class process_group: public command_group {
public:
    /// Constructor, makes handler with specified references
    /// to app-exec and process processors
    process_group(exec_processor & app_proc,
                  process_processor & proc_proc);

    /// Registers app-exec command
    void reg_app_exec_cmd(exec_processor & app_proc,
                          const std::string & name,
                          const std::string & desc);

private:
    /// Registers attach command handler
    void reg_attach(process_processor & proc_proc);

    /// Registers continue command handler
    void reg_continue(process_processor & proc_proc);

    /// Registers detach command handler
    void reg_detach(process_processor & proc_proc);

    /// Registers interrupt command handler
    void reg_interrupt(process_processor & proc_proc);

    /// Registers kill command handler
    void reg_kill(process_processor & proc_proc);

    /// Registers launch command handler
    void reg_launch(process_processor & proc_proc);

    /// Makes result message from async result
    static std::string make_result_msg(const std::string & msg,
                                       const async::result<unsigned long> & res);
};


}


