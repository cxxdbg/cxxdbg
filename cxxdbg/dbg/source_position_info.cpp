// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file source_position_info.cpp
/// Contains implementation of source_position_info class.

#include "source_position_info.hpp"
#include <cassert>


namespace cxxdbg::dbg {


source_position_info::source_position_info():
line_{0} {
}


source_position_info::source_position_info(const std::filesystem::path & p,
                                           unsigned int l):
path_(p),
line_(l) {
    assert(!path_.empty() && "Empty source path is not allowed");
}


const std::filesystem::path &source_position_info::path() const {
    return path_;
}


unsigned int source_position_info::line() const {
    return line_;
}


bool source_position_info::is_valid() const {
    return !path_.empty();
}


source_position_info::operator bool() const {
    return is_valid();
}


bool source_position_info::operator!() const{
    return !is_valid();
}


}
