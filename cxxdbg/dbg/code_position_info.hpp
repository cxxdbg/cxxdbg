// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file code_position_info.hpp
/// Contains definition of code_position_info class.

#pragma once

#include "source_position_info.hpp"
#include <cstdint>
#include <string>


namespace cxxdbg::dbg {


/// \class code_position_info
/// Represents information about position in code.
class code_position_info {
public:
    /// Constructor, makes code position info with specified
    /// address, function name, and source position
    code_position_info(std::uint64_t laddr,
                       std::uint64_t faddr,
                       const std::string & fname,
                       const source_position_info & spos = {});

    /// Returns load address or -1 if load address is not available
    std::uint64_t load_addr() const;

    /// Returns file address
    std::uint64_t file_addr() const;

    /// Returns function name
    const std::string & func_name() const;

    /// Returns source position info
    const source_position_info & src_pos() const;

private:
    std::uint64_t load_addr_;           ///< Code load address
    std::uint64_t file_addr_;           ///< Code file address
    std::string func_name_;             ///< Function name
    source_position_info src_pos_;      ///< Position in source code
};


}


