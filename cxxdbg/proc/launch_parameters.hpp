// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file launch_parameters.hpp
/// Contains definition of the launch_parameters class.

#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>


namespace cxxdbg::proc {


/// Launch parameters
struct launch_parameters {
    std::wstring cmd;                   ///< Full command line
    std::filesystem::path exe;          ///< Path to executable
    std::vector<std::wstring> args;     ///< Arguments
    std::filesystem::path work_dir;     ///< Working directory

    /// Environment variables
    /// TODO: support environment in boost process launchers
    std::map<std::wstring, std::wstring> env;
};


/// Writes parameters to output stream
std::ostream & operator<<(std::ostream & str, const launch_parameters & pars);


}
