// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file watch_group.hpp
/// Contains definition of the watch_group class

#pragma once

#include "break_base_group.hpp"
#include "watch_processor.hpp"
#include "command_group.hpp"


namespace cxxdbg::dbg::cli {


/// Watchpoint command group
class watch_group: public break_base_group {
public:
    /// Constructors watchpoint command group with specified references
    /// to exec processor and watchpoint processor
    watch_group(exec_processor & exec_proc, watch_processor & watch_proc);
};


}


