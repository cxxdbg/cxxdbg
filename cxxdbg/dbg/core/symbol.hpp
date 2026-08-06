// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file symbol.hpp
/// Contains definition of the symbol class.

#pragma once

#include <lldb/API/SBSymbol.h>


namespace cxxdbg::dbg::core {


/// Represents symbol in target
class symbol {
public:
    /// Constructor, makes symbol with specified lldb symbol object
    symbol(const lldb::SBSymbol & s);

    /// Destructor, destroys object
    ~symbol();

    /// Returns pointer to C-string symbol name. Returned pointer
    /// remains valid until while target is loaded.
    const char * name() const;

    /// Returns true if symbol is function
    bool is_func() const;

private:
    lldb::SBSymbol sym_;        ///< lldb symbol object
};


}
