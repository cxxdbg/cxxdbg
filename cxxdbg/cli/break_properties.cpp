// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file break_properties.cpp
/// Contains implementation of the break_properties class
/// and related functions.

#include "break_properties.hpp"


namespace cxxdbg::dbg::cli {


namespace po = boost::program_options;


void add_break_options(const std::string & break_name,
                            boost::program_options::options_description & desc) {
    desc.add_options()
        ("condition,c", po::value<std::string>(),
            (break_name + " condition").c_str())
            ("hit-count,h", po::value<unsigned int>(), "Break if hit count is greater or equal");
}

void process_break_options(const po::variables_map & vars, break_properties & props) {
    // parsing condition
    if (vars.count("condition") > 0) {
        props.cond = vars["condition"].as<std::string>();
    }

    // parsing hit count
    if (vars.count("hit-count") > 0) {
        props.hit_count = vars["hit-count"].as<unsigned int>();
        props.enable_hit_count = props.hit_count > 0;
    }
}


}
