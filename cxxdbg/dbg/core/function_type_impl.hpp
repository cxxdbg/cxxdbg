// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file function_type_impl.hpp
/// Contains definition of the function_type_impl class.

#pragma once

#include "type_impl.hpp"
#include "dbgfmt/backend/function_type_impl.hpp"


namespace cxxdbg::dbg::core {


/// Implementation of function type that reads data from lldb type
class function_type_impl: public type_impl, virtual public dbgfmt::backend::function_type_impl {
public:
    /// Constructs function type implementation
    function_type_impl(target_base & targ, const lldb::SBType & type):
        core::type_impl(targ, type) {}

    /// Returns pointer to return type
    const core::type_impl * ret_type() const override;

    /// Returns number of parameters
    size_t params_size() const override;

    /// Returns type of parameter at specified index
    const core::type_impl * param_at(size_t idx) const override;
};


}
