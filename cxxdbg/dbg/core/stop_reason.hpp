// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file stop_reason.hpp
/// Contains definition of the stop_reason_t enum.

#pragma once


namespace cxxdbg::dbg::core {


/// Target stop reason
enum class stop_reason_t {
    unknown,        ///< Unknown stop reason
    trace,          ///< Tracing
    breakpoint,     ///< Breakpoint
    watchpoint,     ///< Watchpoint
    signal,         ///< Signal received
    exec,           ///< exec called
    crashed         ///< Process crashed
};


}
