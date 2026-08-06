// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file enum_type_impl.cpp
/// Contains implementation of enum_type_impl class.

#include "enum_type_impl.hpp"
#include "dbgfmt/backend/format_error.hpp"
#include <lldb/API/SBError.h>
#include <lldb/API/SBType.h>
#include <lldb/API/SBTypeEnumMember.h>
#include <cassert>


namespace cxxdbg::dbg::core {


enum_type_impl::enum_type_impl(target_base & targ, const lldb::SBType & t):
core::type_impl(targ, t) {
    if (lldb_type().GetTypeClass() != lldb::eTypeClassEnumeration) {
        throw dbgfmt::backend::format_error{"invalid lldb type for enum_value"};
    }
}


std::size_t enum_type_impl::items_size() const {
    lldb::SBTypeEnumMemberList items = lldb_type().GetEnumMembers();
    if (!items.IsValid()) {
        throw dbgfmt::backend::format_error{"can't get lldb enum members"};
    }

    return static_cast<std::size_t>(items.GetSize());
}


std::string enum_type_impl::item_name(std::size_t index) const {
    assert(index < items_size() && "Invalid enum item index");

    lldb::SBTypeEnumMemberList items = lldb_type().GetEnumMembers();
    if (!items.IsValid()) {
        throw dbgfmt::backend::format_error{"can't get lldb enum members"};
    }

    lldb::SBTypeEnumMember item = items.GetTypeEnumMemberAtIndex(static_cast<uint32_t>(index));
    if (!item.IsValid()) {
        throw dbgfmt::backend::format_error{"can't get lldb enum member"};
    }

    const char * nm = item.GetName();
    return nm ? std::string(nm) : std::string();
}


std::int64_t enum_type_impl::item_value(std::size_t index) const {
    assert(index < items_size() && "Invalid enum item index");

    lldb::SBTypeEnumMemberList items = lldb_type().GetEnumMembers();
    if (!items.IsValid()) {
        throw dbgfmt::backend::format_error{"can't get lldb enum members"};
    }

    lldb::SBTypeEnumMember item = items.GetTypeEnumMemberAtIndex(static_cast<uint32_t>(index));
    if (!item.IsValid()) {
        throw dbgfmt::backend::format_error{"can't get lldb enum member"};
    }

    return item.GetValueAsSigned();
}


}
