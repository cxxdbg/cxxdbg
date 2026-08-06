// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file call_parameter.cpp
/// Contains implementation of call_parameter class.

#include "call_parameter.hpp"
#include "call_parameter_info.hpp"


namespace cxxdbg::dbg {


call_parameter::call_parameter(const std::wstring & nm,
                               const std::wstring & tp,
                               const std::wstring & val):
name_{nm},
type_{tp},
value_{val} {
}


call_parameter::call_parameter(const call_parameter_info & pinfo):
name_{pinfo.name()},
type_{pinfo.type()},
value_{pinfo.value()} {
}


const std::wstring & call_parameter::name() const {
    return name_;
}


const std::wstring & call_parameter::type() const {
    return type_;
}


const std::wstring & call_parameter::value() const {
    return value_;
}


}
