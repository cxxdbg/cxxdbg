// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file appcore.cpp
/// Contains implementation of function which creates debugger implementation for
/// core library.

#include "appcore.hpp"
#include "core_debugger_impl.hpp"
#include "dbgfmt/context.hpp"


namespace cxxdbg::dbg::appcore {


dbg::debugger_impl_sp make_debugger_impl(async::event_queue & main_queue) {
    return dbg::debugger_impl_sp(new core_debugger_impl(main_queue));
}


}
