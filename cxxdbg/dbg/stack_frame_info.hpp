// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file stack_frame_info.hpp
/// Contains definition of stack_frame_info class.

#pragma once

#include "call_parameter_info.hpp"
#include "code_position_info.hpp"
#include <vector>


namespace cxxdbg::dbg {


/// \class stack_frame_info
/// Contains information about stack frame in call stack
class stack_frame_info {
public:
    /// Type of vector of parameter infos
    typedef std::vector<call_parameter_info> call_parameter_info_vector;

    /// Constructor, makes stack frame info with specified code
    /// position info, CFA and symbol context ID
    stack_frame_info(const code_position_info & sp,
                     std::uint64_t cfap,
                     std::uint64_t s_id);

    /// Returns code position info
    const code_position_info & pos() const;

    /// Returns CFA for stack frame
    uint64_t cfa() const;

    /// Returns symbol context if for stack frame
    uint64_t sc_id() const;

    /// Returns const reference to vector of parameter infos
    const call_parameter_info_vector & params() const;

    /// Adds parameter info to the list of parameters
    void add_param(const call_parameter_info & par);

private:
    code_position_info pos_;                ///< Code position info
    std::uint64_t cfa_;                     ///< CFA
    std::uint64_t sc_id_;                   ///< Symbol context ID (for inlined functions)
    call_parameter_info_vector params_;     ///< Call parameters infos
};


}


