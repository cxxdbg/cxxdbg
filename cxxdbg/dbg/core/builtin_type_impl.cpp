// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file builtin_type_impl.cpp
/// Contains implementation of builtin_type_impl class.

#include "builtin_type_impl.hpp"
#include "dbgfmt/backend/format_error.hpp"
#include <lldb/API/SBError.h>
#include <cassert>
#include <sstream>


namespace cxxdbg::dbg::core {


builtin_type_impl::builtin_type_impl(target_base & targ, const lldb::SBType & t):
core::type_impl(targ, t) {
    auto tcls = lldb_type().GetTypeClass();
    if (tcls != lldb::eTypeClassBuiltin &&
        tcls != lldb::eTypeClassComplexFloat &&
        tcls != lldb::eTypeClassComplexInteger) {

        throw dbgfmt::backend::format_error{"lldb value type is not builtin"};
    }
}


builtin_type_impl::kind_t builtin_type_impl::kind() const {
    auto tcls = lldb_type().GetTypeClass();

    // special case for complex types
    // TODO: cross platform complex type detection
    if (tcls == lldb::eTypeClassComplexInteger) {
        switch (size()) {
        case 2:
            return kind_t::complex_char_;
        case 4:
            return kind_t::complex_short_;
        case 8:
            return kind_t::complex_int_;
        case 16:
            return kind_t::complex_long_long_;
        default: {
            std::ostringstream msg;
            msg << "Unknown complex size: " << size();
            throw dbgfmt::backend::format_error{msg.str()};
        }
        }
    } else if (tcls == lldb::eTypeClassComplexFloat) {
        switch (size()) {
        case 8:
            return kind_t::complex_float_;
        case 16:
            return kind_t::complex_double_;
        default:
            return kind_t::complex_long_double_;
        }
    }

    assert(tcls == lldb::eTypeClassBuiltin && "invalid type class for builtin type");

    switch (lldb_type().GetBasicType()) {
    case lldb::eBasicTypeVoid:
        return kind_t::void_;
    case lldb::eBasicTypeChar:
        return kind_t::char_;
    case lldb::eBasicTypeSignedChar:
        return kind_t::char_;
    case lldb::eBasicTypeUnsignedChar:
        return kind_t::unsigned_char_;
    case lldb::eBasicTypeWChar:
        return kind_t::wchar_t_;
    case lldb::eBasicTypeSignedWChar:
        return kind_t::wchar_t_;
    case lldb::eBasicTypeUnsignedWChar:
        return kind_t::wchar_t_;
    case lldb::eBasicTypeChar16:
        return kind_t::char16_t_;
    case lldb::eBasicTypeChar32:
        return kind_t::char32_t_;
    case lldb::eBasicTypeShort:
        return kind_t::short_;
    case lldb::eBasicTypeUnsignedShort:
        return kind_t::unsigned_short_;
    case lldb::eBasicTypeInt:
        return kind_t::int_;
    case lldb::eBasicTypeUnsignedInt:
        return kind_t::unsigned_int_;
    case lldb::eBasicTypeLong:
        return kind_t::long_;
    case lldb::eBasicTypeUnsignedLong:
        return kind_t::unsigned_long_;
    case lldb::eBasicTypeLongLong:
        return kind_t::long_long_;
    case lldb::eBasicTypeUnsignedLongLong:
        return kind_t::unsigned_long_long_;
    case lldb::eBasicTypeBool:
        return kind_t::bool_;
    case lldb::eBasicTypeFloat:
        return kind_t::float_;
    case lldb::eBasicTypeDouble:
        return kind_t::double_;
    case lldb::eBasicTypeLongDouble:
        return kind_t::long_double_;
    case lldb::eBasicTypeFloatComplex:
        return kind_t::complex_float_;
    case lldb::eBasicTypeDoubleComplex:
        return kind_t::complex_double_;
    case lldb::eBasicTypeLongDoubleComplex:
        return kind_t::complex_long_double_;
    case lldb::eBasicTypeNullPtr:
        return kind_t::nullptr_t_;
    default: {
        std::ostringstream msg;
        msg << "Unknown LLDB builtin type kind "
            << static_cast<int>(lldb_type().GetBasicType());
        throw dbgfmt::backend::format_error{msg.str()};
    }
    }
}


}
