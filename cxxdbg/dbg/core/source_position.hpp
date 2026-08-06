// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file source_position.hpp
/// Contains definition of source_position class.

#pragma once

#include <lldb/API/SBLineEntry.h>

#include <filesystem>
#include <string>


namespace cxxdbg::dbg::core {


/// \class source_position
/// Contains information about source file name and position.
class source_position {
public:
    /// Default constructor, makes invalid source position info
    source_position();

    /// Constructor, makes source position info with specified
    /// file path, line, and column
    source_position(const std::filesystem::path & p, unsigned int l);

    /// Returns source file path
    const std::filesystem::path & path() const;

    /// Returns line number
    unsigned int line() const;

    /// Returns true if source position is valid
    bool is_valid() const;

    /// Returns true if source position is valid
    explicit operator bool() const;

    /// Returns true if source position is not valid
    bool operator!() const;

    /// Creates source position from lldb line entry
    static source_position from_line_entry(lldb::SBLineEntry le);

    /// Returns true if source position is equal to other
    bool operator==(const source_position & pos) const {
        return path() == pos.path() && line() == pos.line();
    }

    /// Returns true if source posotion is not equal to other
    bool operator!=(const source_position & pos) const {
        return !(*this == pos);
    }

private:
    std::filesystem::path path_;  ///< Source file path
    unsigned int line_;             ///< Line number
};


}
