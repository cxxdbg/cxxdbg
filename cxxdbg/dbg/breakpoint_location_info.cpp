// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file breakpoint_loction_info.cpp
/// Contains implementation of breakpoint_location_info class.

#include "breakpoint_location_info.hpp"


namespace cxxdbg::dbg {


breakpoint_location_info::breakpoint_location_info(id_type i,
                                                   const code_position_info & p):
id_(i),
pos_(p) {
}


breakpoint_location_info::id_type breakpoint_location_info::id() const {
    return id_;
}


const code_position_info & breakpoint_location_info::pos() const {
    return pos_;
}


}
