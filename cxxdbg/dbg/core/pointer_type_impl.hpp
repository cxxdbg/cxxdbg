// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file pointer_type_impl.hpp
/// Contains definition of pointer_type_impl class.

#pragma once

#include "type_impl.hpp"
#include "dbgfmt/backend/pointer_type_impl.hpp"


namespace cxxdbg::dbg::core {


/// Represents implementation of pointer type
class pointer_type_impl: public type_impl, public dbgfmt::backend::pointer_type_impl {
public:
    /// Constructor, makes value with specified pointer to lldb value
    pointer_type_impl(target_base & targ, const lldb::SBType & t);

    /// Destructor, destroys object
    virtual ~pointer_type_impl();
};


}
