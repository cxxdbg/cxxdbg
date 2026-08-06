// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file lldb_pointer_type_impl.cpp
/// Contains implementation of lldb_pointer_type_impl class.

#include "lldb_pointer_type_impl.hpp"
#include "target_base.hpp"
#include "type_utils.hpp"
#include "dbgfmt/backend/format_error.hpp"
#include <lldb/API/SBError.h>
#include <lldb/API/SBTarget.h>
#include <sstream>


namespace cxxdbg::dbg::core {


lldb_pointer_type_impl::lldb_pointer_type_impl(target_base & targ, const lldb::SBType & t):
pointer_type_impl(targ, t) {
    lldb::TypeClass tpc = lldb_type().GetTypeClass();
    if (tpc != lldb::eTypeClassPointer &&
        tpc != lldb::eTypeClassReference &&
        tpc != lldb::eTypeClassMemberPointer)
        throw dbgfmt::backend::format_error{"lldb type is not pointer or reference"};
}


lldb_pointer_type_impl::~lldb_pointer_type_impl() {
}


bool lldb_pointer_type_impl::is_reference() const {
    return lldb_type().GetTypeClass() == lldb::eTypeClassReference;
}


const type_impl * lldb_pointer_type_impl::base() const {
    return targ().get_or_create_type(lldb_type().GetPointeeType());
}


bool lldb_pointer_type_impl::can_dereference() const {
    auto tp = lldb_type();

    if (tp.GetTypeClass() == lldb::eTypeClassMemberPointer) {
        // can't dereference pointer to member
        return false;
    }

    // getting pointee type
    auto ptp = lldb_type_pointee(tp);

    // untypedefing type
    if (ptp.IsTypedefType()) {
        ptp = ptp.GetTypedefedType();
    }

    if (!ptp.IsValid()) {
        throw dbgfmt::backend::format_error{"can't get lldb pointee type"};
    }

    if (ptp.GetTypeClass() == lldb::eTypeClassBuiltin &&
        ptp.GetBasicType() == lldb::eBasicTypeVoid) {
        // can't dereference pointer to void
        return false;
    }

    if (ptp.GetTypeClass() == lldb::eTypeClassFunction) {
        // can't dereference pointer to function
        return false;
    }

    return true;
}


}
