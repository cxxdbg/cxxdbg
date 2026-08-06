// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file source_position.cpp
/// Contains implementation of source_position class.

#include "source_position.hpp"
#include <cassert>


namespace cxxdbg::dbg::core {


source_position::source_position():
line_{0} {
}


source_position::source_position(const std::filesystem::path & p,
                                           unsigned int l):
path_(p),
line_(l) {
    assert(!path_.empty() && "Empty source path is not allowed");
}


const std::filesystem::path &source_position::path() const {
    return path_;
}


unsigned int source_position::line() const {
    return line_;
}


bool source_position::is_valid() const {
    return !path_.empty();
}


source_position::operator bool() const {
    return is_valid();
}


bool source_position::operator!() const{
    return !is_valid();
}


source_position source_position::from_line_entry(lldb::SBLineEntry le) {
    if (!le.IsValid()) {
        // no source position available
        return source_position();
    }

    // get file path from line entry

    std::string src_path;
    lldb::SBFileSpec src_spec = le.GetFileSpec();
    auto dir_cstr = src_spec.GetDirectory();
    if (dir_cstr != nullptr) {
        src_path = dir_cstr;
    }

    auto file_cstr = src_spec.GetFilename();
    if (file_cstr != nullptr) {
        src_path += "/";
        src_path += file_cstr;
    }

    // Workaround for bug in lldb: lldb sometimes can't correctly read debug
    // info and returns empty source file path. In that case we return
    // invalid source position
    if (src_path.empty()) {
        return source_position();
    }

    // make source position info
    return source_position(src_path, le.GetLine());
}


}
