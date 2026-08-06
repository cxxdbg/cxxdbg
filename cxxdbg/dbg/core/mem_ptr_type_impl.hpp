// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file mem_ptr_type_impl.hpp
/// Contains definition of the mem_ptr_type_impl class.

#pragma once

#include "record_type_impl.hpp"
#include "type_impl.hpp"
#include "dbgfmt/backend/mem_ptr_type_impl.hpp"


namespace cxxdbg::dbg::core {


/// Implementation of the dbgfmt::backend::mem_ptr_type_impl interface for core
class mem_ptr_type_impl: public type_impl, virtual public dbgfmt::backend::mem_ptr_type_impl {
public:
    /// Constructs member pointer type impl with specified reference to taret
    /// and pointer to lldb type
    mem_ptr_type_impl(target_base & targ, const lldb::SBType & t);

    /// Returns pointer to type of member
    const core::type_impl * mem_type() const override;

    /// Returns pointer to type of object
    const record_type_impl * obj_type() const override;
};


}
