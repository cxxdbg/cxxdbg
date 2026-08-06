// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file builtin_type_impl.hpp
/// Contains definition of builtin_type_impl class.

#pragma once

#include "type_impl.hpp"
#include "dbgfmt/backend/builtin_type_impl.hpp"


namespace cxxdbg::dbg::core {


/// Represents implementation of builtin type in debugger
class builtin_type_impl: public type_impl, public dbgfmt::backend::builtin_type_impl {
public:
    /// Constructors builtin type implementation
    builtin_type_impl(target_base & targ, const lldb::SBType & t);

    /// Returns kind of builtin type
    kind_t kind() const override;
};


}
