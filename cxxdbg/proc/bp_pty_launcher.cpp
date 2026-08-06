// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file bp_pty_launcher.cpp
/// Contains implementation of the bp_pty_launcher class.

#include "async_pty.hpp"
#include "bp_pty_launcher.hpp"
#include "bp_utils.hpp"
#include "child_pty.hpp"

#include "cxxdbg/boost_process/process.hpp"


namespace bp = boost::process;


namespace cxxdbg::proc {


bp_pty_launcher::pty_bp_child_monitor::pty_bp_child_monitor(boost::asio::io_context & io_c,
                                                            const boost::asio::executor & e_exec,
                                                            boost::process::child && bp_child,
                                                            pty_ptr && pty_p):
base_type(io_c, e_exec, std::move(bp_child), *pty_p, *pty_p),
pty_{std::move(pty_p)} {
}


std::unique_ptr<child_io_monitor> bp_pty_launcher::launch_io(const launch_parameters & pars) {
    CXXDBG_PROC_LOG_INFO << "launch pty: " << convert_launch_pars_to_string(pars);

    // creating PTY
    auto pty = std::make_unique<async_pty>(io_ctx_);

    // launching process
    auto bp_child = create_bp_child(pars, child_pty(*pty));

    // creating child object
    auto chld = std::make_unique<pty_bp_child_monitor>(io_ctx_,
                                                       event_exec_,
                                                       std::move(bp_child),
                                                       std::move(pty));

    // starting reading of stdout and stderr from child
    chld->start_read_stdout();

    return chld;
}


}
