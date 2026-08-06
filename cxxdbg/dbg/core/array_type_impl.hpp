// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file array_type_impl.hpp
/// Contains definition of array_type_impl class.

#pragma once

#include "type_impl.hpp"
#include "dbgfmt/backend/array_type_impl.hpp"


namespace cxxdbg::dbg::core {


/// \class array_value_impl
/// Represents implementation of value of array type in debugger
class array_type_impl: public type_impl, public dbgfmt::backend::array_type_impl {
public:
    /// Constructor, makes array value with specified pointer to lldb value
    array_type_impl(target_base & targ, const lldb::SBType & tp);

    /// Destructor, destroys object
    virtual ~array_type_impl();

    /// Returns pointer to implementation of base type
    const core::type_impl * base() const override;

    /// Returns size of array
    std::size_t array_size() const override;
};


}
