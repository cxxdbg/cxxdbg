// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file module_info.hpp
/// Contains definition of module_info class.

#pragma once

#include "compile_unit_info.hpp"
#include <set>


namespace cxxdbg::dbg {


class module_impl;
typedef std::shared_ptr<module_impl> module_impl_sp;


/// \class module_info
/// Represents info about single module in debug info
class module_info {
    /// Type of vector of compile units
    typedef std::vector<compile_unit_info> compile_unit_info_vector;

    /// Type of set of source files
    typedef std::set<std::filesystem::path> source_file_set;

public:
    /// Constructor, makes module info with specified path and shared pointer
    /// to module implementation
    module_info(const std::filesystem::path & p,
                const module_impl_sp & impl);

    /// Returns path to module
    const std::filesystem::path & mod_path() const;

    /// Adds compile unit to module
    void add_compile_unit(const compile_unit_info & cu);

    /// Returns reference to vector of compile units
    const compile_unit_info_vector & compile_units() const;

    /// Adds source file to module info
    void add_source(const std::filesystem::path & p);

    /// Returns const reference set of source files
    const source_file_set & sources() const;

    /// Returns shared pointer to module implementation
    const module_impl_sp & mod_impl() const;

    /// Returns reference to sorted vector of function names
    std::vector<const char*> & funcs();

    /// Returns const reference to sorted vector of function names
    const std::vector<const char*> & funcs() const;

private:
    std::filesystem::path mpath_;         ///< Path to module
    compile_unit_info_vector cunits_;       ///< Vector of compile units
    source_file_set sources_;               ///< Set of sources
    module_impl_sp mod_impl_;               ///< Module implementation

    /// Sorted vector of function names
    std::vector<const char*> funcs_;
};


}


