// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file json_settings_storage.cpp
/// Contains implementation of the json_settings_storage class.

#include "json_settings_storage.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>


namespace cxxdbg {

using json_t = nlohmann::json;


json_settings_storage::json_settings_storage(const std::filesystem::path & p):
json_path_{p} {
    // opening json file
    std::ifstream str{p.string()};
    if (!str.is_open()) {
        // can't open file. Config is empty
        return;
    }

    // reading json from file
    json_t js;
    if (!(str >> js)) {
        std::ostringstream msg;
        msg << "can't read json from config file " << p;
        throw std::runtime_error{msg.str()};
    }

    // adding values to cache
    mem_cache_ = js.get<std::map<std::string, std::string>>();
}


bool json_settings_storage::read_string(const std::string & name, std::string & val) const {
    // reading data from memory cache
    auto res = mem_cache_.find(name);
    if (res == mem_cache_.end())
        return false;

    val = res->second;
    return true;
}


void json_settings_storage::write_string(const std::string & name, const std::string & val) {
    // writing data to memory cache
    mem_cache_[name] = val;

    // dumping memory storage to json file
    dump_data();
}


void json_settings_storage::dump_data() {
    // saving all data to json
    json_t js = mem_cache_;

    // opening json file
    std::ofstream str{json_path_.string()};
    if (!str.is_open()) {
        std::ostringstream msg;
        msg << "can't open config file " << json_path_ << " for writing";
        throw std::runtime_error{msg.str()};
    }

    // writing jsong to file
    if (!(str << js)) {
        std::ostringstream msg;
        msg << "can't write json to file " << json_path_;
        throw std::runtime_error{msg.str()};
    }
}


}
