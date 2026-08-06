// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file global_variable.hpp
/// Contains definition of the global_variable class.

#pragma once

#include "type_impl.hpp"
#include "dbgfmt/backend/debug_object_impl.hpp"


namespace lldb_private {
    class Variable;
}


namespace cxxdbg::dbg::core {


/// Represents global variable in debugger
class global_variable: public dbgfmt::backend::debug_object_impl {
public:
    /// Constructs global variable object with specified lldb variable object
    /// and address in memory
    global_variable(target_base & targ, lldb_private::Variable * var, uint64_t a);

    /// Returns address of global varialbe
    uint64_t addr() const override { return addr_; }

    /// Returns name of global variable
    std::string name() const override;

    /// Returns type of object
    type_impl * type() const override;

    /// Returns position of definition of global variable
    dbgfmt::backend::source_position def_pos() const override;

    /// Always returns null
    const dbgfmt::backend::function_debug_object_impl * as_function() const override {
        return nullptr;
    }

private:
    target_base & targ_;
    lldb_private::Variable * var_;
    uint64_t addr_;
};


}
