// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file lldb_debugger_ext.cpp
/// Contains implementation of functions for extending lldb functionality.

#include "lldb_debugger_ext.hpp"
#include "lldb_debugger_ext_base.hpp"
#include "regex_lists_defaults.hpp"
#include "tn_function_name_parser.hpp"
#include "cxxdbg/dbg/core/regex_lists.hpp"
#include "cxxdbg/util/string_vector.hpp"
#include <sstream>


namespace cxxdbg::dbg::core {


void init_lldb_extensions() {
    // setting custom function name parser
    set_lldb_custom_function_name_parser(std::make_unique<tn_function_name_parser>());
}


/// Converts list of list of regular expressions to string taking into account enabled flag
static std::string regex_lists_to_string(const regex_lists & lists) {
    std::vector<std::string> regex_list;

    for (auto && list : lists) {
        // skipping disabled values
        if (!list.enabled()) {
            continue;
        }

        for (auto && r : list.data()) {
            regex_list.push_back(r);
        }
    }

    return util::string_vector_to_string_spaces(regex_list);
}


void init_lldb_extensions_instance(const lldb::SBDebugger & dbg, bool set_default_regexps) {
    // setting default values for step-avoid-regexp and step-through-regexp settings
    if (set_default_regexps) {
        auto avoid_regex = regex_lists_to_string(functions_to_skip::default_value());
        set_lldb_setting_value(dbg, "target.process.thread.step-avoid-regexp", avoid_regex);

        auto step_through_regex = regex_lists_to_string(functions_to_step_into::default_value());
        set_lldb_setting_value(dbg, "target.process.thread.step-through-regexp", step_through_regex);
    }
}



}
