// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file bp_pty_launcher.hpp
/// Contains definition of the bp_pty_launcher class.

#pragma once

#include "async_pty.hpp"
#include "bp_io_child_monitor.hpp"
#include "io_launcher.hpp"


namespace cxxdbg::proc {


/// Process launcher implementation via boost.process and boost.asio that
/// creates new PTY for launched process.
class bp_pty_launcher: public io_launcher {
public:
    /// Child process object that redirects stdin/stderr/stdout to PTY
    class pty_bp_child_monitor: public basic_bp_io_child_monitor<async_pty, async_pty> {
        friend class bp_pty_launcher;

        using pty_ptr = std::unique_ptr<async_pty>;
        using base_type = basic_bp_io_child_monitor<async_pty, async_pty>;

    public:
        /// Constructs child object with boost process child and PTY
        pty_bp_child_monitor(boost::asio::io_context & io_c,
                             const boost::asio::executor & e_exec,
                             boost::process::child && bp_child,
                             pty_ptr && pty_p);

    private:
        pty_ptr pty_;
    };


    /// Constructs launcher with specified reference to IO context and
    /// executor for posting events
    bp_pty_launcher(boost::asio::io_context & ioc, boost::asio::executor event_exec):
        io_ctx_{ioc}, event_exec_{std::move(event_exec)} {}

    /// Launches child process in PTY. Returns object associated with launched process.
    std::unique_ptr<child_io_monitor> launch_io(const launch_parameters & pars) override;

private:
    boost::asio::io_context & io_ctx_;      ///< Reference to IO context
    boost::asio::executor event_exec_;      ///< Executor for posting events
};


}
