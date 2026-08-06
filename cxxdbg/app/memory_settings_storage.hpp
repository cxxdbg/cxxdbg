// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file memory_settings_storage.hpp
/// Contains definition of the memory_settings_storage class.

#pragma once

#include "cxxdbg/app/settings_storage.hpp"
#include <map>


namespace cxxdbg {


/// settings_storage implementation that stores data in memory
class memory_settings_storage: public settings_storage {
public:
    /// Writes string value with specified name to storage
    void write_string(const std::string & name, const std::string & val) override;

    /// Reads string value with specified name from storage. Returns true
    /// if storage contains value.
    bool read_string(const std::string & name, std::string & val) const override;

private:
    std::map<std::string, std::string> settings_;       ///< Map of settings values
};


}
