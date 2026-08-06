// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file module_impl.hpp
/// Contains definition of the module_impl class.

#pragma once

#include <memory>


namespace cxxdbg::dbg {


/// Interface for module implementation
class module_impl {
public:
    /// Destructor, destroys object
    virtual ~module_impl() {}
};


/// Type of shared pointer to module implementation
typedef std::shared_ptr<module_impl> module_impl_sp;


}


