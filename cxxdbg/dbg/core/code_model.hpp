// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file code_model_impl.hpp
/// Contains definition of the code_model_impl class.

#pragma once

#include <string>


namespace cxxdbg::dbg::core {


/// Interface for code model implementation that core module
/// uses for parsing function and type names
class code_model {
public:
    /// Destroys object
    virtual ~code_model() = default;

    /// Transforms type name to canonical forms that is used in debugging info in gcc and clang
    /// Returns true on success
    virtual bool make_canonical_type_names(const std::string & t,
                                           std::string & gcc_can_name,
                                           std::string & clang_can_name) const = 0;

    /// Parses function name and writes function name without return type
    /// and parameters into specified output variables. Returns true on success
    virtual bool parse_function_name(const std::string & name,
                                     std::string & name_no_ret_type,
                                     std::string & name_no_params) const = 0;
};


}
