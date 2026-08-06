// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file bp_terminal_launcher.hpp
/// Contains definition of the terminal_launcher type alias.

#pragma once

#ifdef _WIN32
#include "bp_io_launcher.hpp"
#else
#include "bp_pty_launcher.hpp"
#endif


namespace cxxdbg::proc {
#ifdef _WIN32
    // TODO: implement launching in terminal for Windows
    using bp_terminal_launcher = bp_io_launcher;
#else
    using bp_terminal_launcher = bp_pty_launcher;
#endif
}
