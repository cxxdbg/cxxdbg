// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file compile_unit_info.hpp
/// Contains defintiion of compile_unit_info class.

#pragma once

#include <filesystem>
#include <vector>


namespace cxxdbg::dbg {


/// \class compile_unit_info
/// Represents info about single compile unit in debug info
class compile_unit_info {
    /// Type of vector of paths
    typedef std::vector<std::filesystem::path> path_vector;

public:
    /// Constructor, makes compile unit info with specified path to source
    compile_unit_info(const std::filesystem::path & src);

    /// Returns path to source
    const std::filesystem::path & source_path() const;

private:
    std::filesystem::path source_;            ///< Path to source
};


}


