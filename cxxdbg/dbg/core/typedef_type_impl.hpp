// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file typedef_type_impl.hpp
/// Contains definition of typedef_type_impl class.

#pragma once

#include "type_impl.hpp"
#include "dbgfmt/backend/typedef_type_impl.hpp"


namespace cxxdbg::dbg::core {


/// Represents implementation of typedef type
class typedef_type_impl: public type_impl, public dbgfmt::backend::typedef_type_impl {
public:
    /// Constructor, makes typedef value with specified pointer
    /// to lldb value
    typedef_type_impl(target_base & targ, const lldb::SBType & t);

    /// Destructor, destroys object
    virtual ~typedef_type_impl();

    /// Returns pointer to untypedefed value
    core::type_impl * base() const override;
};


}
