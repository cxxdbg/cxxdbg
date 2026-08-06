// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file module.hpp
/// Contains definition of the module class.

#pragma once

#include <memory>
#include <vector>


namespace cxxdbg::dbg {


class module_impl;
class module_info;
typedef std::shared_ptr<module_impl> module_impl_sp;


/// Represents module in executable being debugged
class module {
public:
    /// Constructor, makes module from module info
    module(const module_info & mod_inf);

    /// Find functions with names with specified prefix
    void find_funcs(const std::string & prefix, std::vector<const char *> & res) const;

private:
    module_impl_sp impl_;               ///< Shared pointer to module implementation
    std::vector<const char*> funcs_;    ///< Sorted vector of module functions
};


}


