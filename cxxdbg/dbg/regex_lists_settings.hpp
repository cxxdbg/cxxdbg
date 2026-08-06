// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file regex_lists_settings.hpp
/// Contains declaration of functions for loading and saving regex lists from/to settings storage.

#pragma once

#include "cxxdbg/app/settings_storage.hpp"


namespace cxxdbg::dbg::core {
    class regex_lists;
}


namespace cxxdbg::dbg {


/// loads regex lists from settings
core::regex_lists load_regex_lists_from_settings(const settings_storage & settings,
                                                 const std::string & storage_name,
                                                 const core::regex_lists & default_value);


/// saves regex lists to settings
void save_regex_lists_to_settings(settings_storage & settings,
                                  const std::string & storage_name,
                                  const core::regex_lists & lists);


}
