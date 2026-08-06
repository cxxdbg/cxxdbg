// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

//
// Created by yzarudny on 13.12.17.
//

#pragma once

#include <cassert>
#include <codecvt>
#include <string>
#include <vector>


namespace cxxdbg::util::convert {


inline std::wstring to_wstring(const char * start, const char * end) {
    if (start == end) {
        return {};
    }

    // creating multibyte -> wide converter using current locale
    std::locale loc("");
    using conv_t = std::codecvt<wchar_t, char, std::mbstate_t>;
    const conv_t & conv = std::use_facet<conv_t>(loc);

    // allocating output buffer.
    // output will take at most same number of bytes as input
    std::vector<wchar_t> res(end - start);

    // converting chars to wchars
    std::mbstate_t state = mbstate_t();
    wchar_t * output = &res[0];
    const char * next_input = start;
    wchar_t * next_output = output;
    conv.in(state,
            start,
            end,
            next_input,
            output,
            output + res.size(),
            next_output);

    // creating and returning wstring
    return std::wstring(output, next_output);
}


inline std::wstring to_wstring(const std::string & s) {
    return to_wstring(s.data(), s.data() + s.size());
}


inline std::wstring to_wstring(const std::vector<char> & v) {
    return to_wstring(v.data(), v.data() + v.size());
}


inline std::vector<char> to_char_vector(std::wstring_view s) {
    // creating multibyte -> wide converter using current locale
    std::locale loc("");
    using conv_t = std::codecvt<wchar_t, char, std::mbstate_t>;
    const conv_t & conv = std::use_facet<conv_t>(loc);

    // allocating output buffer.
    // output will take at most same number of bytes as input
    std::vector<char> res(s.size() * 4 + 4);

    // converting chars to wchars
    std::mbstate_t state = mbstate_t();
    const wchar_t * input = s.data();
    const wchar_t * input_end = input + s.size();
    char * output = &res[0];
    const wchar_t * next_input = input;
    char * next_output = output;

    // converting wchars to chars
    conv.out(state,
             input,
             input_end,
             next_input,
             output,
             output + res.size(),
             next_output);

    // resising resulting array to output size
    assert(next_output - output <= res.size() && "invalid output size");
    res.resize(next_output - output);

    return res;
}


inline std::vector<char> to_char_vector(const std::wstring & s) {
    return to_char_vector(std::wstring_view{s});
}


inline std::string to_string(const std::wstring & s) {
    auto res = to_char_vector(s);
    return std::string(res.begin(), res.end());
}


}

