// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file launch_parameters.cpp
/// Contains implementation of the launch_parameters class.

#include "launch_parameters.hpp"
#include "cxxdbg/util/convert.hpp"


namespace cxxdbg::proc {


std::ostream & operator<<(std::ostream & str, const launch_parameters & pars) {
    if (!pars.cmd.empty()) {
        str << util::convert::to_string(pars.cmd);
    } else {
        str << pars.exe.string();
        if (!pars.args.empty()) {
            str << "[";
            bool first = true;
            for (auto && arg : pars.args) {
                if (first) {
                    first = false;
                } else {
                    str << ", ";
                }

                str << util::convert::to_string(arg);
            }
            str << "]";
        }
    }

    return str;
}


}
