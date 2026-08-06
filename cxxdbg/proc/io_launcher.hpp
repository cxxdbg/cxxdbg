// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file io_launcher.hpp
/// Contains definition of the io_launcher class.

#pragma once

#include "child_io_monitor.hpp"
#include "launcher.hpp"


namespace cxxdbg::proc {


/// Process launcher that supports IO interaction with launched process
class io_launcher: public launcher {
public:
    /// Virtual destructor
    virtual ~io_launcher() = default;

    /// Launches child process for IO. Returns IO monutor assoicated with launched process.
    virtual std::unique_ptr<child_io_monitor> launch_io(const launch_parameters & pars) = 0;

    /// Launches child process. Returns IO monitor associated with launched process.
    std::unique_ptr<monitor> launch(const launch_parameters & pars) override {
        return launch_io(pars);
    }
};


}
