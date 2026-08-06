// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file paths.hpp
/// Contains declarations of functions for getting various paths

#pragma once

#include <filesystem>


namespace cxxdbg::util {


/// Returns path to home directory
std::filesystem::path home_path();

/// Returns path to data directory. Creates directory if does not exist.
std::filesystem::path data_dir_path();


}
