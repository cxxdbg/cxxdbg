// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file vector_type_impl.cpp
/// Contains implementation of vector_type_impl class.

#include "vector_type_impl.hpp"
#include "target_base.hpp"
#include "dbgfmt/backend/format_error.hpp"
#include <lldb/Symbol/Type.h>


namespace cxxdbg::dbg::core {


vector_type_impl::vector_type_impl(target_base & targ, const lldb::SBType & t):
core::type_impl(targ, t) {
    auto tp = lldb_type();

    if (!tp.IsValid()) {
        throw dbgfmt::backend::format_error("can't get lldb vector value type");
    }

    if (tp.GetTypeClass() != lldb::eTypeClassVector)
        throw dbgfmt::backend::format_error("invalid lldb type for vector value");
}


vector_type_impl::~vector_type_impl() {
}


const type_impl *vector_type_impl::base() const {
    return targ().get_or_create_type(lldb_type().GetVectorElementType());
}


std::size_t vector_type_impl::vector_size() const {
    auto tp = lldb_type();
    auto type_impl = tp.GetSP();
    uint64_t size = 0;
    type_impl->GetCompilerType(true).IsVectorType(nullptr, &size);
    return size;
}


}
