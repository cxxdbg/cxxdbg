// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file tn_code_model.hpp
/// Contains definition of the tn_code_model class.

#pragma once

#include "code_model.hpp"
#include <cxxtn/tn.hpp>


namespace cxxdbg::dbg::core {


/// Code model implementation for core module that uses type name library
/// to parse functions and type names
class tn_code_model: public code_model {
    /// Transforms type name to canonical form that is used in debugging info.
    /// Returns true on success
    bool make_canonical_type_names(const std::string & t,
                                   std::string & gcc_can_name,
                                   std::string & clang_can_name) const override {
        return cxxtn::make_canonical_type_names(t, gcc_can_name, clang_can_name);
    }

    /// Parses function name and writes function name without return type
    /// and parameters into specified output variables. Returns true on success
    bool parse_function_name(const std::string & name,
                             std::string & name_no_ret_type,
                             std::string & name_no_params) const override {
        return cxxtn::parse_function_name(name, name_no_ret_type, name_no_params);
    }
};


}
