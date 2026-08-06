// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file vector_type_impl.hpp
/// Contains definition of vector_type_impl class.

#pragma once

#include "type_impl.hpp"
#include "dbgfmt/backend/vector_type_impl.hpp"


namespace cxxdbg::dbg::core {


/// \class vector_type_impl
/// Represents implementation of value of vector type in debugger
class vector_type_impl: public type_impl, public dbgfmt::backend::vector_type_impl {
public:
    /// Constructor, makes array value with specified pointer to lldb value
    vector_type_impl(target_base & targ, const lldb::SBType & tp);

    /// Destructor, destroys object
    virtual ~vector_type_impl();

    /// Returns pointer to implementation of base type
    const core::type_impl * base() const override;

    /// Returns size of vector
    std::size_t vector_size() const override;
};


}
