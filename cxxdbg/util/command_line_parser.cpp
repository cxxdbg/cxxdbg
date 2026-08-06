// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "command_line_parser.hpp"
#include <algorithm>


namespace cxxdbg { namespace util {



void split_command_line(std::istream & str, std::vector<std::string> & res) {

    bool inside_quote = false;
    bool inside_apos = false;
    bool were_quotes = false;
    std::string cur_arg;
    std::locale loc;

    while (true) {
        char c = static_cast<char>(str.get());

        if (!str)
            break;

        if (std::isspace(c, loc)) {
            if (inside_quote || inside_apos) {
                // we are inside quotes, put space to current arg
                cur_arg.push_back(c);
            } else {
                // end of command argument
                // putting curent argument to the list of args if
                // it's not empty or there were quotes
                if (!cur_arg.empty() || were_quotes) {
                    res.push_back(cur_arg);
                    cur_arg.clear();
                    were_quotes = false;
                }
            }
        } else if (c == '\"') {
            if (inside_quote) {
                // end of quotes area
                inside_quote = false;
                were_quotes = true;
            } else if (inside_apos) {
                // process quote as simple char
                cur_arg.push_back(c);
            } else {
                // begining of quotes area
                inside_quote = true;
            }
        } else if (c == '\'') {
            if (inside_apos) {
                // end of apos area
                inside_apos = false;
                were_quotes = true;
            } else if (inside_quote) {
                // process apos as simple char
                cur_arg.push_back(c);
            } else {
                // begining of apos area
                inside_apos = true;
            }
        } else if (c == '\\') {
            // reading next character
            char c2 = static_cast<char>(str.get());
            if (!str) {
                // end of input reached after escape character \,
                // processing \ as simple character
                cur_arg.push_back(c);
                break;
            } else {
                cur_arg.push_back(c2);
            }
        } else {
            // simple character
            cur_arg.push_back(c);
        }
    }

    // adding last arg to result
    if (!cur_arg.empty() || were_quotes) {
        res.push_back(cur_arg);
    }
}


void write_command_line(std::ostream & str, const std::vector<std::string> & args) {
    std::locale loc;

    bool first = true;

    for (auto arg : args) {
        // writting separator
        if (!first) {
            str << ' ';
        } else {
            first = false;
        }

        // checking for spaces
        auto res = std::find_if(arg.begin(), arg.end(), [&loc](char c) {
            return std::isspace(c, loc);
        });
        bool has_spaces = res != arg.end();

        if (has_spaces || arg.empty()) {
            str << '\"';
        }

        // processing characters
        for (auto c : arg) {
            if (c == '\\') {
                str << "\\\\";
            } else if (c == '\"') {
                str << "\\\"";
            } else if (c == '\'') {
                // escape apos if no quotes
                if (!has_spaces) {
                    str << '\\';
                }
                str << c;
            } else {
                str << c;
            }
        }

        if (has_spaces || arg.empty()) {
            str << '\"';
        }
    }
}


} }
