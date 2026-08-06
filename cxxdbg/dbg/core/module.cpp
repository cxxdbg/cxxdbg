// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file module.cpp
/// Contains implementation of module class.

#include "module.hpp"
#include "utils.hpp"


namespace cxxdbg::dbg::core {


module::module(const lldb::SBModule & m):
mod_(m) {
}


module::module(const module & m):
mod_{m.mod_} {
}


module::~module() {
}


std::filesystem::path module::path() const {
    return lldb_sb_file_spec_to_path(mod_.GetFileSpec());
}


std::size_t module::compile_units_size() const {
    lldb::SBModule m = mod_;
    return static_cast<std::size_t>(m.GetNumCompileUnits());
}


compile_unit module::compile_unit_at(std::size_t i) const {
    assert(i < compile_units_size() && "Invalid compile unit index");
    lldb::SBModule m = mod_;
    return compile_unit(m.GetCompileUnitAtIndex(static_cast<uint32_t>(i)));
}


size_t module::symbols_size() const {
    lldb::SBModule m = mod_;
    return m.GetNumSymbols();
}


symbol module::symbol_at(size_t i) const {
    lldb::SBModule m = mod_;
    return m.GetSymbolAtIndex(i);
}


void module::reindex() {
    // making debug info indexes by calling FindFirstType function
    mod_.FindTypes("std::string");

    // making symbol indexes by callind FindFunctions function
    mod_.FindFunctions("main");
}


}
