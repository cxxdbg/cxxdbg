// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// file debugger_state.hpp
/// Contains definition of the debugger_state enum.

#pragma once


namespace cxxdbg::dbg {


/// Debugger state
enum class debugger_state {
    initial,        ///< Initial application state, target is not loaded
    connecting,     ///< Connecting to platform now
    loading,        ///< Target is loading now
    loaded,         ///< Target is loaded
    unloading,      ///< Target is unloading now
    launching,      ///< Target is launching
    running,        ///< Target is running
    stopped,        ///< Target is stopped (interrupted)
    terminating,    ///< Target is being terminated
    detaching       ///< Detaching from target
};


}
