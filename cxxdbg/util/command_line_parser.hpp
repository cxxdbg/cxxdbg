// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file command_line_parser.hpp
/// Contains definition of functions for extended command line parser
/// which splits regular options from --args'-like option list

#pragma once

#include <istream>
#include <vector>
#include <string>


namespace cxxdbg { namespace util {


/// Prses command line, splits regular options from --args'-like option list.
/// Returns true if --args option is found
template <typename Iterator>
bool parse_args_command_line(Iterator it,
                             Iterator end,
                             std::vector<std::string> & regular_options,
                             std::vector<std::string> & args_options) {

    bool args_parsed = false;

    static const std::string s_args = "--args";
    static const std::string s_dash = "--";

    for (; it != end; ++it) {
        if (args_parsed) {
            args_options.push_back(*it);
        } else {
            if (s_args == *it || s_dash == *it) {
                args_parsed = true;
            } else {
                regular_options.push_back(*it);
            }
        }
    }

    return args_parsed;
}


/// Reads command line from input stream and splits it into vector of arguments
void split_command_line(std::istream & str, std::vector<std::string> & res);

/// Writes vector of command line arguments as command line to output stream
void write_command_line(std::ostream & str, const std::vector<std::string> & args);


} }


