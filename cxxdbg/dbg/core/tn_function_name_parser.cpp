// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file tn_function_name_parser.cpp
/// Contains implementation of the tn_function_name_parser class.

#include "tn_function_name_parser.hpp"
#include <cxxtn/tn.hpp>


namespace cxxdbg::dbg::core {


bool tn_function_name_parser::parse(const std::string & name,
                                    std::string & name_no_ret_type,
                                    std::string & name_no_params) const {
    return cxxtn::parse_function_name(name, name_no_ret_type, name_no_params);
}


}
