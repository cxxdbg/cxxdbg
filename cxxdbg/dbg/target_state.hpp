// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file target_state.hpp
/// Contains definition of the target_state enum.

#pragma once


namespace cxxdbg::dbg {


enum class target_state {
    invalid,        ///< Target is invalid (unloaded)
    loaded,         ///< Target is loaded, but not started
    launching,      ///< Target is launching
    running,        ///< Target is running
    stopped,        ///< Target is stopped (interrupted)
    terminating,    ///< Target is being terminated
    detaching,      ///< Detaching from target
    unloading       ///< Target is being unloaded
};


}
