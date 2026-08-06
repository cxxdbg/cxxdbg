// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file paths.cpp
/// Contains implementations of functions for getting various paths

#include "paths.hpp"


namespace fs = std::filesystem;

namespace cxxdbg::util {


fs::path home_path() {
#ifdef _WIN32
        auto home_dir_var_name = "USERPROFILE";
#else
        auto home_dir_var_name = "HOME";
#endif
        auto p = getenv(home_dir_var_name);
        if (p == nullptr) {
            throw std::runtime_error{"can't get path to home directory"};
        }

        return p;
}


fs::path data_dir_path() {
    auto dir = home_path() / ".cxxdbg";
    if (!fs::exists(dir)) {
        fs::create_directories(dir);
    }

    return dir;
}


}
