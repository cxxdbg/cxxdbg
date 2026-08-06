// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/*
 * File:   regex_lists_defaults.hpp
 * Author: extremer
 *
 * Created on September 6, 2016, 12:34 PM
 */

#pragma once

#include "regex_lists.hpp"

namespace cxxdbg::dbg::core {

namespace functions_to_skip {
/// creates and returns default value for the names of functions to skip when debugging
regex_lists default_value();

/// defaul storage name
const std::string & default_storage_name();
}

namespace functions_to_step_into {
/// creates and returns default value for the names of functions to skip when debugging
regex_lists default_value();

/// defaul storage name
const std::string & default_storage_name();
}

namespace functions_to_group {
/// creates and returns default value for the names of functions to group into subtree when in callstack model
regex_lists default_value();

/// defaul storage name
const std::string & default_storage_name();

}
}
