// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file print.hpp
/// Contains definition of printing utilities

#pragma once

#include <format>
#include <ostream>
#include <sstream>
#include <type_traits>


namespace cxxdbg { namespace util {


/// Prints integer as hex value with specified size with no prefix
template <typename C, typename T>
static void print_hex_no_prefix(std::basic_ostream<C> & str,
                                T x,
                                std::size_t sz,
                                std::size_t min_sz) {

    // converting value to unsigned type
    auto ux = std::make_unsigned_t<T>(x);

    // clearing bits higher than requested size
    decltype(ux) mask = 0xFF;
    if constexpr (sizeof(mask) > 1) {
        for (size_t i = 1; i < sz; ++i) {
            mask <<= 8;
            mask |= static_cast<decltype(ux)>(0xFF);
        }
    }
    ux &= mask;

    std::size_t real_min_sz = min_sz;

    if (min_sz > sz) {
        // use type size as minimum size
        real_min_sz = sz;
    } else {
        // detecting if value fits into minimum size
        decltype(ux) max_val = 255;
        for (auto i = 1; i < min_sz; ++i) {
            max_val *= 255;
        }

        if (x <= max_val) {
            real_min_sz = min_sz;
        } else {
            real_min_sz = sz;
        }
    }

    if constexpr (std::is_same_v<C, char>) {
        str << std::format("{:0{}X}", ux, real_min_sz * 2);
    } else {
        str << std::format(L"{:0{}X}", ux, real_min_sz * 2);
    }
}


/// Prints character as hex value of specified size with no prefix.
/// We need this because of special meaning of operator<< overload
/// for std::ostream and char
inline void print_hex_no_prefix(std::ostream &str,
                                char x,
                                std::size_t sz,
                                std::size_t min_sz) {
    print_hex_no_prefix(str, static_cast<int>(x), sz, min_sz);
}


/// Prints integer as hex value of specified size with 0x prefix
template <typename C, typename T>
static void print_hex(std::basic_ostream<C> & str,
                      T x,
                      std::size_t sz,
                      std::size_t min_sz) {
    str.put(static_cast<C>('0'));
    str.put(static_cast<C>('x'));

    print_hex_no_prefix(str, x, sz, min_sz);
}


/// Parses unsigned integer from string (hex or dec)
template <typename T>
bool parse_hex_uint(const std::string & str, T & val) {
    // checking for empty string
    if (str.empty())
        return false;

    // trying parse 0x hex
    if (str.size() > 2 && str[0] == '0' && str[1] == 'x') {
        std::istringstream s{str};
        s.get();
        s.get();

        s >> std::hex >> val;
        return !s.fail() && s.eof();
    }

    // parsing decimal
    std::istringstream s{str};
    s >> val;
    return !s.fail() && s.eof();
}



} }


