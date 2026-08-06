// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file regex.cpp
/// Contains implementation of functions for checking regular expressions.

#include "regex.hpp"
#include <lldb/Utility/RegularExpression.h>
#include <llvm/ADT/StringRef.h>
#include <cassert>
#include <vector>


namespace cxxdbg::dbg::core {


std::string validate_regex(const std::string & regex) {
    if (regex.empty())
        return "regular expression is empty";

    lldb_private::RegularExpression r{llvm::StringRef(regex)};
    if (r.IsValid()) {
        return {};
    }

    auto err = r.GetError();
    if (!err) {
        return {};
    }

    std::string err_str;
    llvm::handleAllErrors(std::move(err), [&err_str](const llvm::ErrorInfoBase & err) {
        err_str = err.message();
    });

    return "can't parse regular expression: " + err_str;
}


}
