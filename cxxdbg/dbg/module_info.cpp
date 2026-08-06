// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file module_info.cpp
/// Contains implementation of module_info class.

#include "module_info.hpp"


namespace cxxdbg::dbg {


module_info::module_info(const std::filesystem::path & p,
                         const module_impl_sp & impl):
mpath_{p},
mod_impl_{impl} {
}


const std::filesystem::path & module_info::mod_path() const {
    return mpath_;
}


void module_info::add_compile_unit(const compile_unit_info & cu) {
    cunits_.push_back(cu);
}


const module_info::compile_unit_info_vector & module_info::compile_units() const {
    return cunits_;
}


void module_info::add_source(const std::filesystem::path & p) {
    sources_.insert(p);
}


const module_info::source_file_set & module_info::sources() const {
    return sources_;
}


const module_impl_sp & module_info::mod_impl() const {
    return mod_impl_;
}


std::vector<const char *> & module_info::funcs() {
    return funcs_;
}


const std::vector<const char*> & module_info::funcs() const {
    return funcs_;
}


}
