// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file call_parameter_info.hpp
/// Contains definition of call_parameter_info class.

#pragma once

#include <string>


namespace cxxdbg::dbg {


/// \class call_parameter_info
/// Contains information about call parameter in stack frame
class call_parameter_info {
public:
    /// Constructor, makes new call parameter info
    call_parameter_info(const std::wstring & nm,
                        const std::wstring & tp,
                        const std::wstring & val);


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


