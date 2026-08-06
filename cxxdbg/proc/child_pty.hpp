// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file child_pty.hpp
/// Contains definition of the child_pty class.

#pragma once

#include "async_pty.hpp"
#include "cxxdbg/boost_process/process.hpp"
#include <iostream>


namespace cxxdbg::proc {


/// Boost process initializer that setups PTY for launched child process
class child_pty: public boost::process::extend::handler {
public:
    /// Constructs pty initializer for boost process library
    child_pty(async_pty & pty):
        pty_{pty} {}

    template<typename Executor> void on_setup(Executor &) const {
    }

    template<typename Executor>
    void on_error(Executor &, const std::error_code &) const {
    }

    template<typename Executor> void on_success(Executor &) const {
    }

    template<typename Executor>
    void on_fork_error(Executor &, const std::error_code &) const {
    }

    template<typename Executor> void on_exec_setup(Executor &) const {
        ::setsid();

        // opening slave PTY
        auto slave_fd = pty_.open_slave();

//        // Acquire the controlling terminal
//        if (::ioctl(slave_fd, TIOCSCPTY, (char *)0) < 0) {
//            // TODO
//        }

        // redirectring stdin/stout/stderr to slave PTY
        ::close(STDIN_FILENO);
        ::close(STDOUT_FILENO);
        ::close(STDERR_FILENO);
        dup2(slave_fd, STDIN_FILENO);
        dup2(slave_fd, STDOUT_FILENO);
        dup2(slave_fd, STDERR_FILENO);
    }

    template<typename Executor>
    void on_exec_error(Executor &, const std::error_code &) const {
    }

private:
    async_pty & pty_;       ///< Reference to PTY
};


}
