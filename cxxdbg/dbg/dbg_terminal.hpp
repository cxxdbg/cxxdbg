// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file dbg_terminal.hpp
/// Contains definition of the dbg_terminal class.

#pragma once

#include "cxxdbg/app/signals.hpp"
#include "cxxdbg/app/terminal.hpp"


namespace cxxdbg::dbg {


class application;
class target;


/// Represents executable terminal in debugger application
class dbg_terminal: public terminal {
    friend class debugger;

public:
    /// Sends stdin to process being debugged
    void send_stdin(const std::string & data) override;

    /// Returns true if terminal can send stdin
    bool can_send_stdin() const override;

private:
    /// Connects to target
    void connect(target * targ);

    /// Disconnects from current target
    void disconnect();


    target * targ_ = nullptr;           ///< Pointer to current target
    bool was_running_ = false;          ///< true if target was running
};


}


