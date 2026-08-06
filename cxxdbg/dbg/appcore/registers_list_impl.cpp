// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file registers_list_impl.cpp
/// Contains implementation of the registers_list_impl class.

#include "registers_list_impl.hpp"
#include "utils.hpp"
#include "cxxdbg/dbg/core/registers.hpp"
#include "cxxdbg/dbg/core/target.hpp"
#include "cxxdbg/util/convert.hpp"


namespace cxxdbg::dbg::appcore {


std::map<registers_list_impl::watch_id, dbgfmt::named_fmt_result>
registers_list_impl::get_variables() {
    // returning empty list if taget is not in stopped state
    if (targ().state() != core::target::state_t::stopped) {
        return {};
    }

    // getting current stack frame
    core::stack_frame frame = targ().thread_at(targ().current_thread_index()).selected_frame();

    // Workaround for CXXDBG-231:
    // sometimes lldb returns invalid frame for valid index,
    // that happens for start_thread frame
    if (!frame.is_valid())
        return {};

    // reading registers
    auto registers = frame.read_registers();

    // building map of register groups
    std::map<watch_id, dbgfmt::named_fmt_result> vals;
    for (size_t i = 0, sz = registers.groups_size(); i < sz; ++i) {
        auto grp = registers.group_at(i);
        auto grp_name = util::convert::to_wstring(grp.name());
        auto fmt_res = registers.group_fmt_result_at(i);
        vals.insert({i, fmt_res});
    }
    return vals;
}


}
