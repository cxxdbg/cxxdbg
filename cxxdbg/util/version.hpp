// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file version.hpp
/// Contains declarations of utility structures and functions for working with version strings.

#pragma once

#include <ostream>
#include <string_view>


namespace cxxdbg::util {


/// Contains version information
struct version {
    int major;
    int minor;
    int patch;

    /// Returns true if all version components are zeros
    bool is_zero() const {
        return major == 0 && minor == 0 && patch == 0;
    }

    /// Converts version to string
    std::string string() const;
};

/// Returns true if two versions are equal
bool operator==(const version & v1, const version & v2);

/// Returns true if two versions are different
inline bool operator!=(const version & v1, const version & v2) {
    return !(v1 == v2);
}

/// Returns true if v1 is less than v2
bool operator<(const version & v1, const version & v2);

/// Writes version to output stream
template <typename Char>
std::basic_ostream<Char> & operator<<(std::basic_ostream<Char> & str, const version & v) {
    return str << v.major << Char('.') << v.minor << Char('.') << v.patch;
}

/// Parses version from string. Returns zero version if it can't be parsed.
version parse_version(std::string_view s);


}
