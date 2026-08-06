// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file function_debug_object_impl.hpp
/// Contains definition of the function_debug_object_impl class.

#pragma once

#include "dbgfmt/backend/debug_object_impl.hpp"
#include <lldb/API/SBFunction.h>


namespace cxxdbg::dbg::core {


class target_base;


/// Implementation of debug_object_impl interface for lldb function object
class function_debug_object_impl: public dbgfmt::backend::function_debug_object_impl {
public:
    /// Constructs debug object implementation with specified lldb function object
    function_debug_object_impl(target_base & t, const lldb::SBFunction & f):
        targ_{t}, func_{f} {}

    /// Returns address of object
    uint64_t addr() const override;

    /// Returns name of object
    std::string name() const override;

    /// Returns type of object
    dbgfmt::backend::type_impl * type() const override;

    /// Returns position of definition of object
    dbgfmt::backend::source_position def_pos() const override;

    /// Returns type of record for member function
    dbgfmt::backend::record_type_impl * record_type() const override;

    /// Returns this
    const function_debug_object_impl * as_function() const override {
        return this;
    }

private:
    /// Returns copy of stored reference lldb function object
    lldb::SBFunction func() const { return func_; }

    target_base & targ_;        ///< Reference to target
    lldb::SBFunction func_;     ///< LLDB function object
};


}
