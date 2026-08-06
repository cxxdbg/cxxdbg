// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file settings_storage.cpp
/// Contains implementations of functions for loading/saving regex lists from/to settings storage.

#include "regex_lists_settings.hpp"
#include "core/regex_lists.hpp"


namespace cxxdbg::dbg {


core::regex_lists load_regex_lists_from_settings(const settings_storage & settings,
                                                 const std::string & storage_name,
                                                 const core::regex_lists & default_value) {
    const std::string items_count_path = storage_name + "/count";
    size_t items_count = settings.read(items_count_path, 0u);
    
    if (0 == items_count) {
        return default_value;
    }

    core::regex_lists res;
    
    for (size_t i = 0; i < items_count; ++i) {
        const std::string item_base_path = storage_name + "/" + std::to_string(i);
        
        const std::string item_name_path = item_base_path + "/name";
        const std::string item_data_path = item_base_path + "/data";
        const std::string item_enabled_path = item_base_path + "/enabled";
        const std::string & default_name = item_base_path;
        
        std::string item_name = settings.read(item_name_path, default_name);
        
        //regex_list_item::container_type default_value();
        core::regex_list_item::container_type item_data = std::move(
                settings.read<typename core::regex_list_item::container_type>(item_data_path, {})
        );
        
        const int enabled_default_value = 0; // false
        int enabled = settings.read(item_enabled_path, enabled_default_value);
        
        // create item
        core::regex_list_item item(item_name);
        item.enable(enabled == 1);
        
        for (const auto & it : item_data) {
            item.add(it);
        }
        
        res.add(item);  
    }
    
    return res;
}


void save_regex_lists_to_settings(settings_storage & settings,
                                  const std::string & storage_name,
                                  const core::regex_lists & lists) {

    const std::string items_count_path = storage_name + "/count";
    size_t items_count = lists.size();
    
    settings.write(items_count_path, items_count);
    
    for (size_t i = 0; i < items_count; ++i) {
        const std::string item_base_path = 
            storage_name + "/" + std::to_string(i);
        
        const std::string item_name_path = item_base_path + "/name";
        const std::string item_data_path = item_base_path + "/data";
        const std::string item_enabled_path = item_base_path + "/enabled";
        
        const core::regex_list_item & item = lists[i];
        
        settings.write(item_name_path, item.name());
        settings.write<typename core::regex_list_item::container_type>(item_data_path, item.data());
        settings.write(item_enabled_path, item.enabled() ? 1 : 0);
    }
}


}
