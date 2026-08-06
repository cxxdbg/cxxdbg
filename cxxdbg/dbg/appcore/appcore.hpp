// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file appcore.hpp
/// Contains definition of function which creates debugger implementation for
/// core library.

#pragma once

#include "cxxdbg/dbg/forward.hpp"
#include "cxxdbg/async/forward.hpp"


namespace cxxdbg::dbg::appcore {


/// Makes debugger implementation for core library
dbg::debugger_impl_sp make_debugger_impl(async::event_queue & main_queue);


}


