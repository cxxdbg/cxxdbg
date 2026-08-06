// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread_utils.hpp
/// Contains implementation of utility functions for working with threads.

#pragma once

#include "debugger.hpp"
#include "target.hpp"
#include "thread_list.hpp"


namespace cxxdbg::dbg {


/// Returns range of threads located at specified position
inline auto threads_at_line(const debugger & dbg, const source_position & pos) {
    auto fn = [pos](auto && thrd) {
        if (std::ranges::empty(thrd->call_stack())) {
            // no call stack info in thread
            return false;
        }

        const stack_frame * frame = thrd->call_stack().front();

        if (!frame->pos().src_pos()) {
            // no source position is available
            return false;
        }

        if (frame->pos().src_pos() != pos) {
            return false;
        }

        return true;
    };

    auto threads = (dbg.state() != debugger::state_t::stopped) ?
                   dbg.current_target().threads().all_threads() :
                   thread_list::empty_threads();

    return threads | std::ranges::views::filter(fn);
}


}


