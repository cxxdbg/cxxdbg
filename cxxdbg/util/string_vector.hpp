// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file string_vector.hpp
/// Contains declaration of utility function for converting vectors to strings.

#pragma once

#include <vector>
#include <string>


namespace cxxdbg::util {


/// Concatenates values from vector to single string with space separator.
/// Escapes spaces in values with backslash.
std::string string_vector_to_string_spaces(const std::vector<std::string> & values);


}
