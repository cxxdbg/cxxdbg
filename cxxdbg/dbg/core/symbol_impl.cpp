// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file symbol_impl.cpp
/// Contains implementation of the symbol_impl class.

#include "symbol_impl.hpp"
#include "target_base.hpp"


namespace cxxdbg::dbg::core {


uint64_t symbol_impl::addr() const {
    return static_cast<uint64_t>(sym().GetStartAddress().GetLoadAddress(targ_.lldb_targ()));
}


std::string symbol_impl::name() const {
    auto name_str = sym().GetName();
    if (name_str) {
        return name_str;
    }

    return {};
}


uint64_t symbol_impl::size() const {
    uint64_t start = addr();
    uint64_t end = static_cast<uint64_t>(sym().GetEndAddress().GetLoadAddress(targ_.lldb_targ()));
    return end - start;
}


}
