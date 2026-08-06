// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file memory_settings_storage.cpp
/// Contains implementation of the memory_settings_storage class.

#include "memory_settings_storage.hpp"


namespace cxxdbg {


bool memory_settings_storage::read_string(const std::string & name, std::string & val) const {
    auto res = settings_.find(name);
    if (res == settings_.end())
        return false;

    val = res->second;
    return true;
}


void memory_settings_storage::write_string(const std::string & name, const std::string & val) {
    settings_[name] = val;
}


}
