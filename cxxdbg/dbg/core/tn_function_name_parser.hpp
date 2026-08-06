// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file tn_function_name_parser.hpp
/// Contains definition of tn_function_name_parser class.

#pragma once

#include "function_name_parser.hpp"


namespace cxxdbg::dbg::core {


/// Custom function name parser that uses type name libarary for parsing
class tn_function_name_parser: virtual public function_name_parser {
public:
    bool parse(const std::string & name,
               std::string & name_no_ret_type,
               std::string & name_no_params) const override;
};


}
