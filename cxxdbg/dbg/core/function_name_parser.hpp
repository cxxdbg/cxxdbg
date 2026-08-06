// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file function_name_parser.hpp
/// Contains definition of the function_name_parser class.

#pragma once

#include <string>


namespace cxxdbg::dbg::core {


/// Abstract interface for setting up custom function name parser in LLDB
class function_name_parser {
public:
    /// Default virtual destructor
    virtual ~function_name_parser() = default;

    /// Parses function name
    virtual bool parse(const std::string & name, std::string & name_no_ret_type, std::string & name_no_params) const = 0;
};


}
