// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file version.cpp
/// Contains implementation of utility functions for working with version strings.

#include "version.hpp"
#include <ranges.hpp>
#include <sstream>


namespace cxxdbg::util {


std::string version::string() const {
    std::ostringstream str;
    str << *this;
    return str.str();
}


bool operator==(const version & v1, const version & v2) {
    return v1.major == v2.major && v1.minor == v2.minor && v1.patch == v2.patch;
}


bool operator<(const version & v1, const version & v2) {
    if (v1.major < v2.major) {
        return true;
    } else if (v1.major == v2.major) {
        if (v1.minor < v2.minor) {
            return true;
        } else if (v1.minor == v2.minor) {
            return v1.patch < v2.patch;
        }
    }

    return false;
}


/// Parses integer from range of characters
template <typename Range>
bool parse_int(const Range & chars, int & res) {
    auto c_chars = chars | std::ranges::views::common;
    std::istringstream str{std::string{std::ranges::begin(c_chars), std::ranges::end(c_chars)}};
    return (str >> res) && str.eof();
}


version parse_version(std::string_view s) {
    // constructing range view that contains ranges of characters of splitted string
    auto v_comps = s | std::ranges::views::split('.');

    // checking that version has 3 components
    if (std::ranges::distance(v_comps) != 3) {
        return {};
    }

    int major = 0, minor = 0, patch = 0;

    auto it = std::ranges::begin(v_comps);
    if (!parse_int(*it, major)) {
        return {};
    }

    ++it;
    if (!parse_int(*it, minor)) {
        return {};
    }

    ++it;
    if (!parse_int(*it, patch)) {
        return {};
    }

    return {major, minor, patch};
}


}
