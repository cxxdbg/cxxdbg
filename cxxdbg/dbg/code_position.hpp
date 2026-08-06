// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file code_position.hpp
/// Contains definition of code_position class.

#pragma once

#include "source_position.hpp"
#include <cstdint>
#include <string>


namespace cxxdbg::dbg {


class code_position_info;


/// \class code_position
/// Represents position in code
class code_position {
public:
    /// Constructor, makes code position with specified address,
    /// function name, and source position
    code_position(std::uint64_t laddr,
                  std::uint64_t faddr,
                  const std::string & fname,
                  const source_position & spos = {});

    /// Converts code position info to code position using specified
    /// reference to source model
    code_position(const code_position_info & pinfo, source_model & sources);

    /// Returns code load address or -1 if address is not available
    std::uint64_t load_addr() const;

    /// Returns true if load address is available
    bool has_load_addr() const;

    /// Returns code file address
    std::uint64_t file_addr() const;

    /// If load address is available then returns it. Else returns
    /// file address
    std::uint64_t addr() const;

    /// Returns address string representation
    std::string addr_str() const;

    /// Returns function name
    const std::string & func_name() const;

    /// Returns position in source code
    const source_position & src_pos() const;

    /// Sets position in source code
    void set_src_pos(const source_position & pos) {
        src_pos_ = pos;
    }

    /// Returns source location string representation for code position,
    /// or function name / address if source position is invali
    std::string src_pos_str() const;

    /// Compares two code positions
    bool operator==(const code_position & pos) const;

    /// Compares two code positions
    bool operator!=(const code_position & pos) const;

private:
    std::uint64_t load_addr_;       ///< Code load address
    std::uint64_t file_addr_;       ///< Code file address
    std::string func_name_;         ///< Function name
    source_position src_pos_;       ///< Position in source code
};


}


