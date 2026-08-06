// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file lldb_debugger_ext_base.cpp
/// Contains implementation of functions for reistering extensions in LLDB debugger.
/// This functions depend on LLVM library.

#include "lldb_debugger_ext_base.hpp"
#include "function_name_parser.hpp"
#include <lldb/Core/Mangled.h>
#include <lldb/API/SBDebugger.h>
#include <lldb/API/SBError.h>


namespace cxxdbg::dbg::core {


/// Implementation of the lldb_private::Mangled::CustomFunctionNameParser interface that uses
/// function_name_parser instance for parsing
class lldb_function_name_parser: virtual public lldb_private::Mangled::CustomFunctionNameParser {
public:
    /// Constructs function name parses with specified unique pointer to function_name_parser instance
    lldb_function_name_parser(std::unique_ptr<function_name_parser> && parser):
        parser_{std::move(parser)} {}

    /// Parses function name
    bool parse(const std::string & name, std::string & name_no_ret_type, std::string & name_no_params) const override {
        return parser_->parse(name, name_no_ret_type, name_no_params);
    }

private:
    std::unique_ptr<function_name_parser> parser_;      ///< Pointer to function name parser
};


void set_lldb_custom_function_name_parser(std::unique_ptr<function_name_parser> && parser) {
    lldb_private::Mangled::SetCustomFunctionNameParser(std::make_shared<lldb_function_name_parser>(std::move(parser)));
}


/// Sets lldb setting for specified instance
void set_lldb_setting_value(const lldb::SBDebugger & dbg,
                            const std::string & name,
                            const std::string & val) {
    auto dbg2 = dbg;
    lldb::SBDebugger::SetInternalVariable(name.c_str(), val.c_str(), dbg2.GetInstanceName());
}


}