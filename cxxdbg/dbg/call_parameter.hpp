// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file call_parameter.hpp
/// Contains definition of call_parameter class.

#pragma once

#include <string>


namespace cxxdbg::dbg {


class call_parameter_info;


/// \class call_parameter
/// Represents call parameter in stack frame
class call_parameter {
public:
    /// Constructor, makes new call parameter
    call_parameter(const std::wstring & nm,
                   const std::wstring & tp,
                   const std::wstring & val);

    /// Constructor, makes call parameter from call parameter info
    call_parameter(const call_parameter_info & pinfo);

    /// Returns parameter name
    const std::wstring & name() const;

    /// Returns parameter type
    const std::wstring & type() const;

    /// Returns parameter value
    const std::wstring & value() const;

private:
    std::wstring name_;         ///< Parameter name
    std::wstring type_;         ///< Parameter type
    std::wstring value_;        ///< Parameter value
};


}


