// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file lldb_debugger_ext.hpp
/// Contains declaration of functions for extending lldb functionality.

#pragma once


namespace lldb {
    class SBDebugger;
}


namespace cxxdbg::dbg::core {

/// Performs global initialiation and registers all cxxdbg extensions in lldb
void init_lldb_extensions();

/// Performs LLDB instance initialization and registers all cxxdbg extensions in it
void init_lldb_extensions_instance(const lldb::SBDebugger & dbg, bool set_default_regexps);

}
