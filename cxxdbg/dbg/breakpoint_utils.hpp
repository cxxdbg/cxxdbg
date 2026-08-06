// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file breakpoint_utils.hpp
/// Contains definitions of breakpoint utility functions

#pragma once

#include "breakpoint_list.hpp"
#include "breakpoint_location.hpp"
#include "code_breakpoint.hpp"
#include "source_file.hpp"
#include "source_position.hpp"
#include "source_position_breakpoint.hpp"


namespace cxxdbg::dbg {


/// Returns range of breakpoints located at specified position
inline auto breakpoints_at_line(const breakpoint_list & bp_list, const source_position & pos) {
    // breakpoint filtering function. Returns false if breakpoint should
    // be filtered out from result
    auto filter_fn = [pos](auto && cbp) {

        bool loc_matches = false;

        if (!std::ranges::empty(cbp->locations())) {
            for (auto && loc : cbp->locations()) {
                if (loc->pos().src_pos() == pos) {
                    loc_matches = true;
                    break;
                }
            }
        } else {
            // if does not have locations then checking for source position
            // breakpoints with same location
            if (auto * spbp = dynamic_cast<const source_position_breakpoint*>(cbp)) {
                if (spbp->pos().line() == pos.line()) {
                    if (spbp->pos().path() == pos.file()->path() ||
                        spbp->pos().path() == pos.file()->path().filename()) {

                        loc_matches = true;
                    }
                }
            }
        }

        return loc_matches;
    };

    return bp_list.code_breakpoints() | std::ranges::views::filter(filter_fn);
}


}


