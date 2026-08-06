// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file lldb_debugger_ext_base.hpp
/// Contains declaration of functions for reistering extensions in LLDB debugger.
/// This functions depend on LLVM library.

#pragma once

#include <memory>


namespace lldb {
    class SBDebugger;
}


namespace cxxdbg::dbg::core {

class function_name_parser;


/// Registers custom function name parser in LLDB
void set_lldb_custom_function_name_parser(std::unique_ptr<function_name_parser> && parser);


/// Sets lldb setting for specified instance
void set_lldb_setting_value(const lldb::SBDebugger & dbg,
                            const std::string & name,
                            const std::string & val);


}
