// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file string_vector.cpp
/// Contains implementation of utility function for converting vectors to strings.

#include "string_vector.hpp"
#include <sstream>


namespace cxxdbg::util {

std::string string_vector_to_string_spaces(const std::vector<std::string> & values) {
    // concatenating values with spaces and escaping all spaces in values
    std::ostringstream str;

    bool first = true;
    for (auto && val : values) {
        // writing separator
        if (first) {
            first = false;
        } else {
            str << ' ';
        }

        // writing value
        for (auto && c : val) {
            // escaping space character
            if (c == ' ') {
                str << '\\';
            }

            str << c;
        }
    }

    return str.str();
}


}
