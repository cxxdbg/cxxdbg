// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file stack_frame.hpp
/// Contains definition of stack_frame class.

#pragma once

#include "call_parameter.hpp"
#include "code_position.hpp"
#include <ostream>
#include <vector>


namespace cxxdbg::dbg {


/// \class stack_frame
/// Represents single stack frame in process being debugged
class stack_frame {
public:
    /// Type of vector of call parameters
    typedef std::vector<call_parameter> call_parameter_vector;

    /// Constructor, makes stack frame with specified code position,
    /// CFA, and symbol context
    stack_frame(const code_position & p, uint64_t pcfa, uint64_t psctx);

    /// Returns source position for stack frame
    const code_position & pos() const;

    /// Returns source position for stack frame
    auto & pos() { return pos_; }

    /// Returns stack frame CFA
    uint64_t cfa() const;

    /// Returns stack frame symbol context
    uint64_t sctx() const;

    /// Returns reference to vector of call parameters
    call_parameter_vector & params();

    /// Returns const reference to vector of call parameters
    const call_parameter_vector & params() const;

    /// Adds call parameter to stack frame
    void add_param(const call_parameter & par);

    /// Prints stack frame location to output stream with display options
    void print(std::wostream & str,
               bool display_param_types,
               bool display_param_names,
               bool display_param_vals) const;

private:
    code_position pos_;             ///< Position for stack frame
    uint64_t cfa_;                  ///< Stack frame CFA
    uint64_t sctx_;                 ///< Stack frame symbol context
    call_parameter_vector params_;  ///< Call parameters
};


}


