// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file break_properties.hpp
/// Contains definition of the break_properties class and related stuff.

#pragma once

#include <string>
#include <boost/program_options.hpp>


namespace cxxdbg::dbg::cli {


/// Contains properties common for breakpoints and watchpoints
struct break_properties {
    bool enable_hit_count;      ///< hit count property enabled?
    unsigned int hit_count;     ///< hit count property
    std::string cond;           ///< Breakpoint condition

    break_properties(): enable_hit_count{false}, hit_count{0} {}
};


/// Registers breakpoint/watchpoint options in boost options description
void add_break_options(const std::string & break_name,
                       boost::program_options::options_description & desc);


/// Processes breakpoint/watchpoint command line options and sets values
/// in break_base_properties structure.
void process_break_options(const boost::program_options::variables_map & vars,
                           break_properties & props);


}


