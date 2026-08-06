// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file source_position.cpp
/// Contains implementation of source_position class.

#include "source_position.hpp"
#include "source_model.hpp"
#include "source_position_info.hpp"
#include "source_file.hpp"
#include <cassert>
#include <sstream>


namespace cxxdbg::dbg {


source_position::source_position():
file_{nullptr},
line_{0} {
}


source_position::source_position(const source_file * f, unsigned l):
file_{f},
line_{l} {
    assert(f != nullptr && "null source file is not allowed");
}


source_position::source_position(const source_position_info & pinfo,
                                 source_model & sources):
file_{pinfo.is_valid() ? sources.source(pinfo.path()) : nullptr},
line_{pinfo.line()} {
}


const source_file * source_position::file() const {
    assert(file_ != nullptr && "null source file pointer");
    return file_;
}


unsigned int source_position::line() const {
    return line_;
}


bool source_position::is_valid() const {
    return file_ != nullptr && line_ != 0;
}


source_position::operator bool() const {
    return is_valid();
}


bool source_position::operator!() const {
    return !is_valid();
}


std::string source_position::string() const {
    // checking for invalid source position
    if (!*this)
        return "<invalid source position>";

    std::ostringstream str;
    str << file()->path().filename().string() << ", line " << line();
    return str.str();
}


bool source_position::operator==(const source_position & pos) const {
    return file_ == pos.file_ && line_ == pos.line_;
}


bool source_position::operator!=(const source_position & pos) const {
    return !(*this == pos);
}


}
