// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file bp_io_launcher.hpp
/// Contains definition of the bp_io_launcher class.

#pragma once

#include "bp_io_child_monitor.hpp"
#include "io_launcher.hpp"
#include "cxxdbg/log/log.hpp"

#include "cxxdbg/boost_process/process.hpp"
#include <thread>

#ifndef _WIN32
#include "async_pty.hpp"
#endif


namespace cxxdbg::proc {


/// Process launcher implementation via boost.process and boost.asio
class bp_io_launcher: virtual public io_launcher {
public:
    /// Child process object that redirects stdin/stdout/stderr to pipes
    class stdio_bp_child_monitor: public basic_bp_io_child_monitor<boost::process::async_pipe,
                                                                   boost::process::async_pipe> {

        friend class bp_io_launcher;

        using base_type = basic_bp_io_child_monitor<boost::process::async_pipe, boost::process::async_pipe>;
        using pipe_ptr = std::unique_ptr<boost::process::async_pipe>;

    public:
        /// Constructs child object with boost process child and io pipes
        stdio_bp_child_monitor(boost::asio::io_context & io_c,
                               const boost::asio::executor & e_exec,
                               boost::process::child && bp_child,
                               pipe_ptr && stdout_p,
                               pipe_ptr && stderr_p,
                               pipe_ptr && stdin_p);

        /// Checks that child process is exited and destroys child object
        ~stdio_bp_child_monitor() override;

        /// Returns true if all buffers were cleared and
        /// process should be considered as exited
        bool all_buffers_cleared() const override;

    private:
        /// Starts reading stderr
        void start_read_stderr();

        pipe_ptr std_out_pipe_;                     ///< Stdout pipe
        pipe_ptr std_err_pipe_;                     ///< Stderr pipe
        pipe_ptr std_in_pipe_;                      ///< Stdin pipe
        std::vector<char> std_err_buffer_;          ///< Buffer for reading stderr
    };

    /// Constructs asio process launcher with specified reference to IO context and
    /// executor for posting events
    bp_io_launcher(boost::asio::io_context & ioc, boost::asio::executor event_exec);

    /// Watis untill all IO operations finished, stops working thread and destroys launcher
    ~bp_io_launcher() override;

    /// Launches child process. Returns object associated with launched process.
    std::unique_ptr<child_io_monitor> launch_io(const launch_parameters & pars) override;

private:
    boost::asio::io_context & io_ctx_;      ///< Reference to IO context
    boost::asio::executor event_exec_;      ///< Executor for posting events
};


}
