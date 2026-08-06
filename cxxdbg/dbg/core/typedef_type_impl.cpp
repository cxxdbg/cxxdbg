// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file typedef_type_impl.cpp
/// Contains implementation of typedef_type_impl class.

#include "typedef_type_impl.hpp"
#include "target_base.hpp"
#include "dbgfmt/backend/format_error.hpp"


namespace cxxdbg::dbg::core {


typedef_type_impl::typedef_type_impl(target_base & targ, const lldb::SBType & t):
core::type_impl(targ, t) {
    if (lldb_type().GetTypeClass() != lldb::eTypeClassTypedef) {
        throw dbgfmt::backend::format_error{"invalid lldb type for typedef value"};
    }
}


typedef_type_impl::~typedef_type_impl() {
}


type_impl * typedef_type_impl::base() const {
    return targ().get_or_create_type(lldb_type().GetTypedefedType());
}


}
