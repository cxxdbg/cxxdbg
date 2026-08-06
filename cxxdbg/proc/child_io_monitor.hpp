// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file child_io_monitor.hpp
/// Contains definition of the child_io_monitor class.

#pragma once

#include "monitor.hpp"


namespace cxxdbg::proc {


/// Child process monitor that support IO exchange with child process
class child_io_monitor: public monitor {
public:
    /// Sends stdin to child process
    virtual void send_stdin(const std::string & data) = 0;

    /// Closes stdin of child process
    virtual void close_stdin() = 0;

    /// The signal is emitted when stdout is received
    boost::signals2::signal<void (const std::string &)> stdout_received;

    /// The signal is emitted when stderr is received
    boost::signals2::signal<void (const std::string &)> stderr_received;
};


}
