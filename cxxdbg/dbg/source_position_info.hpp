// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file source_position_info.hpp
/// Contains definition of source_position_info class.

#pragma once

#include <filesystem>


namespace cxxdbg::dbg {


/// \class source_position_info
/// Contains information about source file name and position.
class source_position_info {
public:
    /// Default constructor, makes invalid source position info
    source_position_info();

    /// Constructor, makes source position info with specified
    /// file path, line, and column
    source_position_info(const std::filesystem::path & p, unsigned int l);

    /// Returns source file path
    const std::filesystem::path & path() const;

    /// Returns line number
    unsigned int line() const;

    /// Returns true if source position is valid
    bool is_valid() const;

    /// Returns true if source position is valid
    operator bool() const;

    /// Returns true if source position is not valid
    bool operator!() const;

private:
    std::filesystem::path path_;    ///< Source file path
    unsigned int line_;             ///< Line number
};


}


