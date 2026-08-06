// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file core_module_impl.hpp
/// Contains definition of the core_module_impl class.

#pragma once

#include "cxxdbg/dbg/module_impl.hpp"
#include "cxxdbg/dbg/core/module.hpp"


namespace cxxdbg::dbg::appcore {


/// Module implementation on top of cxxdbgcore
class core_module_impl: public dbg::module_impl {
public:
    /// Constructor, makes module implementation with specified
    /// core module object
    core_module_impl(const core::module & cm);

    core_module_impl(const core_module_impl &) = delete;
    core_module_impl & operator=(const core_module_impl &) = delete;

private:
    core::module mod_;      ///< Core module
};


/// Type of shared pointer to core_module_impl
typedef std::shared_ptr<core_module_impl> core_module_impl_ptr;


}


