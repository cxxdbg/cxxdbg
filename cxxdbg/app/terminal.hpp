// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file terminal.hpp
/// Contains definition of the terminal class.

#pragma once

#include "cxxdbg/util/signals.hpp"


namespace cxxdbg {


/// Abstract representation of terminal in application
class terminal {
public:
    /// Virtual destructor
    virtual ~terminal() = default;

    /// Sends stdin to process being debugged
    virtual void send_stdin(const std::string & data) = 0;

    /// Returns true if terminal can send stdin
    virtual bool can_send_stdin() const = 0;

    /// Stdout received signal
    CXXDBG_DEFINE_SIGNALX(stdout_received, void (const std::wstring&))

    /// Stderr received signal
    CXXDBG_DEFINE_SIGNALX(stderr_received, void (const std::wstring&))

    /// Can send stdin changed signal. The signal is emitted when
    /// return value of can_send_stdin changed
    CXXDBG_DEFINE_SIGNALX(can_send_stdin_changed, void())

    /// Started signal. The signal is emitted when new process is started
    CXXDBG_DEFINE_SIGNALX(started, void(std::wstring_view))

    /// The signal is emitted on error when starting process
    CXXDBG_DEFINE_SIGNALX(start_error, void (const std::wstring&))

    /// Exited signal. The signal is emitted when process exited with
    /// specified exit code
    CXXDBG_DEFINE_SIGNALX(exited, void(int))
};


}


