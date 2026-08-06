// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file call_parameter_info.cpp
/// Contains implementation of call_parameter_info class.

#include "call_parameter_info.hpp"


namespace cxxdbg::dbg {


call_parameter_info::call_parameter_info(const std::wstring & nm,
                                         const std::wstring & tp,
                                         const std::wstring & val):
name_(nm),
type_(tp),
value_(val) {
}


const std::wstring & call_parameter_info::name() const {
    return name_;
}


const std::wstring & call_parameter_info::type() const {
    return type_;
}


const std::wstring & call_parameter_info::value() const {
    return value_;
}


}
