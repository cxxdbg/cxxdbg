// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file lldb_pointer_type_impl.hpp
/// Contains definition of lldb_pointer_type_impl class

#pragma once

#include "pointer_type_impl.hpp"


namespace cxxdbg::dbg::core {


/// Represents pointer type that holds LLDB pointer type
class lldb_pointer_type_impl: public pointer_type_impl {
public:
    /// Constructor, makes value with specified pointer to lldb value
    lldb_pointer_type_impl(target_base & targ, const lldb::SBType & t);

    /// Destructor, destroys object
    virtual ~lldb_pointer_type_impl();

    /// Returns true if pointer has reference type
    bool is_reference() const override;

    /// Returns pointer to base type implementation
    const core::type_impl * base() const override;

    /// Returns true if pointer can be dereferenced
    bool can_dereference() const override;
};


}
