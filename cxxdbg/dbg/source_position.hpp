// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file source_position.hpp
/// Contains definition of source_position class.

#pragma once

#include <string>


namespace cxxdbg::dbg {


class source_file;
class source_model;
class source_position_info;


/// \class source_position
/// Represents position in source file. Line numbers are started from 1.
class source_position {
public:
    /// Default constructor, makes invalid source position
    source_position();

    /// Constructor, makes source position with specified
    /// source file, line, and column
    source_position(const source_file * f, unsigned l);

    /// Converts source position info to source position using specified
    /// source model
    source_position(const source_position_info & pinfo, source_model & sources);

    /// Returns reference to source file
    const source_file * file() const;

    /// Returns line number
    unsigned int line() const;

    /// Returns true if source position is valid
    bool is_valid() const;

    /// Returns true if source position is valid
    operator bool() const;

    /// Returns true if source position is invalid
    bool operator!() const;

    /// Converts source position to string
    std::string string() const;

    /// Equal comparison
    bool operator==(const source_position & pos) const;

    /// Not equal comparison
    bool operator!=(const source_position & pos) const;

private:
    const source_file * file_;          ///< Pointer to source file
    unsigned int line_;                 ///< Line number
};


}


