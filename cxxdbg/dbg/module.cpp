// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file module.cpp
/// Contains implementation of the module class.

#include "module.hpp"
#include "module_info.hpp"
#include <algorithm>
#include <cstring>


namespace cxxdbg::dbg {


module::module(const module_info & mod_inf):
impl_{mod_inf.mod_impl()},
funcs_{mod_inf.funcs()} {
}


void module::find_funcs(const std::string & prefix, std::vector<const char*> & res) const {
    auto it = std::lower_bound(funcs_.begin(), funcs_.end(), prefix.c_str(),
                               [](const char * s1, const char * s2) {
        return strcmp(s1, s2) < 0 ? true : false;
    });

    auto end = funcs_.end();

    while (it != end) {
        if (strncmp(*it, prefix.c_str(), prefix.size()) != 0)
            break;

        res.push_back(*it);
        ++it;
    }
}


}
