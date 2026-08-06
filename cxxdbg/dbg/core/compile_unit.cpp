// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file compile_unit.cpp
/// Contains implementation of compile_unit class

#include "compile_unit.hpp"
#include "utils.hpp"
#include <lldb/API/SBFileSpec.h>
#include <cassert>


namespace fs = std::filesystem;


namespace cxxdbg::dbg::core {


compile_unit::compile_unit(const lldb::SBCompileUnit cu):
cunit_(cu) {
}


compile_unit::~compile_unit() {
}


std::filesystem::path compile_unit::source_path() const {
    return lldb_sb_file_spec_to_path(cunit_.GetFileSpec());
}


std::size_t compile_unit::support_files_size() const {
    return static_cast<std::size_t>(cunit_.GetNumSupportFiles());
}


std::filesystem::path compile_unit::support_file_at(std::size_t i) const {
    assert(i < support_files_size() && "Invalid support file index");
    return lldb_sb_file_spec_to_path(cunit_.GetSupportFileAtIndex(static_cast<uint32_t>(i)));
}


compile_unit::const_support_file_iterator compile_unit::support_files_begin() const {
    return const_support_file_iterator(*this, 0);
}


compile_unit::const_support_file_iterator compile_unit::support_files_end() const {
    return const_support_file_iterator(*this, support_files_size());
}


compile_unit::const_support_file_iterator::const_support_file_iterator(
        const compile_unit & cu, std::size_t i):
iterator_adaptor_(i),
cu_(cu) {
}


compile_unit::const_support_file_iterator::reference
compile_unit::const_support_file_iterator::dereference() const {
    return cu_.support_file_at(this->base());
}


}
