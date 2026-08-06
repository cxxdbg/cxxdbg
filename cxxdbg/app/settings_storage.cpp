// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file settings_storage.cpp
/// Contains implementation of the settings_storage class.

#include "settings_storage.hpp"


namespace cxxdbg {


settings_storage::~settings_storage() {
}


bool settings_storage_split_vector(const std::string & str, std::vector<std::string> & v) {
    if (str.empty())
        return true;

    // the first character must be ','
    if (str[0] != ',')
        return false;

    std::string cur_item;
    for (auto it = std::next(std::begin(str)), end = std::end(str); it != end; ++it) {
        if (*it == ',') {
            // new item
            v.push_back(cur_item);
            cur_item.clear();;
            continue;
        }

        if (*it == '\\') {
            // escape character
            ++it;
            if (it == std::end(str))
                return false;
        }

        cur_item.push_back(*it);
    }

    v.push_back(cur_item);

    return true;
}


std::string settings_storage_merge_vector(const std::vector<std::string> & v) {
    std::ostringstream str;
    for (auto && s : v) {
        str << ',';
        for (auto && c : s) {
            if (c == ',')
                str << "\\,";
            else if (c == '\\')
                str << "\\\\";
            else
                str << c;
        }
    }

    return str.str();
}


}
