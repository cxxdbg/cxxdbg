// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file regex.hpp
/// Contains definition of functions for checking regular expressions.

#pragma once

#include <string>


namespace cxxdbg::dbg::core {


/// Validates regex correctness. Returns error description or
/// empty string of regex is valid.
std::string validate_regex(const std::string & regex);


}
