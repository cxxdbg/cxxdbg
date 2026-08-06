// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file bp_utils.hpp
/// Contains definition of utility functions for boost process.

#pragma once

#include "launch_parameters.hpp"
#include "log.hpp"
#include "cxxdbg/util/convert.hpp"

#ifdef _WIN32
// workaround for compile error in boost process when including extend.hpp
#include <boost/winapi/handle_info.hpp>
#endif

#include "cxxdbg/boost_process/process.hpp"
#include <cassert>
#include <sstream>


namespace cxxdbg::proc {


/// Generic launch function for boost process
template <bool HasCmd, bool HasWorkDir, typename ... Args>
boost::process::child create_bp_child_impl(const launch_parameters & pars, Args && ... args) {
    struct empty_handler: boost::process::extend::handler {};

    auto cmd_par = [&pars] {
        if constexpr (HasCmd) {
            assert(!pars.cmd.empty() && "empty command line");
            return util::convert::to_string(pars.cmd);
        } else {
            return empty_handler{};
        }
    }();

    auto exe_par = [&pars] {
        if constexpr (HasCmd) {
            return empty_handler{};
        } else {
            assert(!pars.exe.empty() && "empty executable");
            return boost::process::exe = pars.exe.string();
        }
    }();

    auto args_par = [&pars] {
        if constexpr (HasCmd) {
            return empty_handler{};
        } else {
            return boost::process::args = pars.args;
        }
    }();

    auto work_dir_par = [&pars] {
        if constexpr (HasWorkDir) {
            return boost::process::start_dir = pars.work_dir.string();
        } else {
            return empty_handler{};
        }
    }();

    return boost::process::child{cmd_par, exe_par, args_par, work_dir_par, std::forward<Args>(args)...};
}


/// Converts command line exe and parameters to string
static inline std::string convert_launch_pars_to_string(const launch_parameters & pars) {
    std::ostringstream str;

    if (!pars.cmd.empty()) {
        str << "'" << util::convert::to_string(pars.cmd) << "'";
        return str.str();
    }

    str << "'" << pars.exe  << "'";

    if (pars.args.empty()) {
        return str.str();
    }

    str << " [";
    bool first = true;
    for (auto && a : pars.args) {
        if (!first) {
            str << ", ";
        }

        str << util::convert::to_string(a);
        first = false;
    }
    str << "]";

    return str.str();
}


/// Creates boost process child using launch parameters and additional launch arguments
template <typename ... Args>
boost::process::child create_bp_child(const launch_parameters & pars, Args && ... args) {
    if (pars.cmd.empty()) {
        // launching using exe and arguments
        assert(!pars.exe.empty() && "executable file and command line are empty");

        if (pars.work_dir.empty()) {
            return create_bp_child_impl<false, false>(pars, std::forward<Args>(args)...);
        } else {
            return create_bp_child_impl<false, true>(pars, std::forward<Args>(args)...);
        }
    } else {
        // launching using command line
        assert(pars.exe.empty() && "executable file and command line both not empty");
        assert(pars.args.empty() && "arguments and command line both not empty");

        if (pars.work_dir.empty()) {
            return create_bp_child_impl<true, false>(pars, std::forward<Args>(args)...);
        } else {
            return create_bp_child_impl<true, true>(pars, std::forward<Args>(args)...);
        }
    }
}


}
