// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file template.cpp
/// Contains implementation of utility functions for parsing template names.

#include "template.hpp"
#include <sstream>
#include <boost/algorithm/string/trim.hpp>


namespace cxxdbg { namespace util {


void parse_template_name(std::istream & str,
                         std::string & name,
                         std::vector<std::string> & pars) {

    name.clear();
    std::string templ_pars;
    unsigned int templ_depth = 0;

    while (true) {
        char c;
        str.read(&c, 1);
        if (!str) {
            // eof reached
            break;
        }

        if (c == '<') {
            // begin of template parameters
            ++templ_depth;
            templ_pars.push_back(c);
        } else if (c == '>') {
            if (templ_depth > 0) {
                --templ_depth;
                templ_pars.push_back(c);
            } else {
                // parsing error
                break;
            }
        } else if (c == ':') {
            char c2;
            str.read(&c2, 1);
            if (!str) {
                // eof reached
                break;
            }

            if (c2 != ':') {
                // parse error
                break;
            }

            templ_pars.push_back(':');
            templ_pars.push_back(':');

            if (templ_depth > 0) {
                // inside template params
            } else {
                // end of scope
                name += templ_pars;
                templ_pars.clear();
            }
        } else {
            if (templ_depth > 0)
                templ_pars.push_back(c);
            else
                name.push_back(c);
        }
    }


    // parsing template parameters from templ_pars

    auto it = templ_pars.begin();
    auto end = templ_pars.end();
    if (it == end) {
        return;
    }

    if (*it != '<')
        return;
    ++it;

    templ_depth = 0;
    std::string par;
    for (; it != end; ++it) {
        if (*it == '<') {
            ++templ_depth;
            par.push_back(*it);
        } else if (*it == '>') {
            if (templ_depth == 0) {
                // end of parameters
                pars.push_back(boost::algorithm::trim_copy(par));
                break;
            } else {
                --templ_depth;
                par.push_back(*it);
            }
        } else if (*it == ',') {
            if (templ_depth == 0) {
                // end of parameter
                pars.push_back(boost::algorithm::trim_copy(par));
                par.clear();
            } else {
                par.push_back(*it);
            }
        } else {
            par.push_back(*it);
        }
    }
}


} }
