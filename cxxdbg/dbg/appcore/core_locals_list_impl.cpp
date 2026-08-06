// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file core_locals_list_impl.cpp
/// Contains implementation of core_locals_list_impl class.

#include "core_locals_list_impl.hpp"
#include "cxxdbg/dbg/core/format.hpp"
#include "cxxdbg/dbg/core/target.hpp"
#include "cxxdbg/dbg/core/variable.hpp"
#include "dbgfmt/fixed_fmt_result.hpp"


namespace cxxdbg::dbg::appcore {



core_locals_list_impl::core_locals_list_impl(async::execution_queue & exec_queue, core::target & targ):
core_watch_list_impl(exec_queue, targ) {
}


std::map<core_locals_list_impl::watch_id, dbgfmt::named_fmt_result>
core_locals_list_impl::get_variables() {
    // returning empty list if taget is not in stopped state
    if (targ().state() != core::target::state_t::stopped) {
        return {};
    }

    core::stack_frame frame = targ().thread_at(targ().current_thread_index()).selected_frame();

    // Workaround for CXXDBG-231:
    // sometimes lldb returns invalid frame for valid index,
    // that happens for start_thread frame
    if (!frame.is_valid())
        return {};

    auto locals = frame.read_locals();

    std::map<watch_id, dbgfmt::named_fmt_result> vals;
    for (auto var : locals) {
        vals.insert({var->id(), format_variable(targ(), frame, var)});
    }

    return vals;
}

}
