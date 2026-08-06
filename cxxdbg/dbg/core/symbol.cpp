// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file symbol.cpp
/// Contains implementation of the symbol class.

#include "symbol.hpp"


namespace cxxdbg::dbg::core {


symbol::symbol(const lldb::SBSymbol & s):
sym_{s} {
}


symbol::~symbol() {
}


const char * symbol::name() const {
    /// LLDB never frees memory allocated for symbol names
    return sym_.GetName();
}


bool symbol::is_func() const {
    lldb::SBSymbol s = sym_;
    return s.GetType() == lldb::eSymbolTypeCode;
}


}
