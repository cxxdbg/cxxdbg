// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file client_term_exec_stub_launcher.hpp
/// Contains definition of the client_term_exec_stub_launcher class.

#pragma once

#include "cxxdbg/proc/async_launcher.hpp"
#include "cxxdbg/proc/launch_parameters.hpp"


namespace cxxdbg::dbg::dapsrv {

class server_impl;


/// Asynchronous process launcher that launches process in DAP client terminal
/// via the cxxdbg-exec-stub utility, obtains PID via exec stub server, and creates
/// monitor for process with obtained PID.
class client_term_exec_stub_launcher: public proc::async_launcher {
public:
    /// Constructs launcher with specified reference to server implementation
    client_term_exec_stub_launcher(server_impl & s_impl):
        s_impl_{s_impl} {}

    /// Launches child process asynchronously. Calls completion handler with child
    /// monitor after launch is complete
    void launch_async(const proc::launch_parameters & pars, const handler & comp_handler) override;

private:
    server_impl & s_impl_;          ///< Reference to server implementation
};


}
