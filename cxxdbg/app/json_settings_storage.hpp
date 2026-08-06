// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file json_settings_storage.hpp
/// Contains definition of the json_settings_storage class.

#pragma once

#include "settings_storage.hpp"
#include <filesystem>
#include <map>


namespace cxxdbg {


/// Settings storage implementation that stores values in JSON file
class json_settings_storage: public settings_storage {
public:
    /// Constructs storage with specified path to JSON file
    explicit json_settings_storage(const std::filesystem::path & p);

private:
    /// Reads string value with specified name from storage. Returns true
    /// if storage contains value.
    bool read_string(const std::string & name, std::string & val) const override;

    /// Writes string value with specified name to storage
    void write_string(const std::string & name, const std::string & val) override;

    /// Dumps cached data to JSON file
    void dump_data();


    std::filesystem::path json_path_;             ///< Path to JSON file
    std::map<std::string, std::string> mem_cache_;  ///< Cache of config in memory
};


}
