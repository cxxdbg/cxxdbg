// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file symbol_impl.hpp
/// Contains definition of the symbol_impl class.

#pragma once

#include "dbgfmt/backend/symbol_impl.hpp"
#include <lldb/API/SBSymbol.h>


namespace cxxdbg::dbg::core {

class target_base;


/// Implementation of dbgfmt::backend::symbol_impl interface that reads symbol info
/// from lldb symbol
/// TODO: try merge it with symbol class.
class symbol_impl: public dbgfmt::backend::symbol_impl {
public:
    /// Constructs symbol implementation with specified lldb symbol
    symbol_impl(target_base & t, const lldb::SBSymbol & s):
        targ_{t}, sym_{s} {}

    /// Returns address of symbol
    uint64_t addr() const override;

    /// Returns name of symbol
    std::string name() const override;

    /// Returns size of symbol
    uint64_t size() const override;

private:
    lldb::SBSymbol sym() const { return sym_; }

    target_base & targ_;
    lldb::SBSymbol sym_;
};


}
