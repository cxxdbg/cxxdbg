// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file array_type_impl.cpp
/// Contains implementation of array_type_impl class.

#include "array_type_impl.hpp"
#include "target_base.hpp"
#include "dbgfmt/backend/format_error.hpp"


namespace cxxdbg::dbg::core {


array_type_impl::array_type_impl(target_base & targ, const lldb::SBType & t):
core::type_impl(targ, t) {
    auto tp = lldb_type();

    if (!tp.IsValid()) {
        throw dbgfmt::backend::format_error("can't get lldb array value type");
    }

    if (tp.GetTypeClass() != lldb::eTypeClassArray)
        throw dbgfmt::backend::format_error("invalid lldb type for array value");
}


array_type_impl::~array_type_impl() {
}


const type_impl *array_type_impl::base() const {
    return targ().get_or_create_type(lldb_type().GetArrayElementType());
}


std::size_t array_type_impl::array_size() const {
    auto tp = lldb_type();
    auto etp = tp.GetArrayElementType();

    if (!etp.IsValid())
        throw dbgfmt::backend::format_error("can't get lldb array element type");

    return tp.GetByteSize() / etp.GetByteSize();
}


}
