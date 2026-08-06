// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file breakpoint_id.hpp
/// Contains definition of the breakpoint_id class.

#pragma once


namespace cxxdbg::dbg::core {


/// Type of breakpoint type
enum class breakpoint_type {
    code,           ///< Code breakpoint
    watch           ///< Watchpoint
};


/// Type of breakpoint number
using breakpoint_num = int;


/// Breakpoint ID. Contains breakpoint type and number
class breakpoint_id {
public:
    /// Breakpoint type
    using type_t = breakpoint_type;

    /// Type of breakpoint number
    using num_t = breakpoint_num;

    /// Constructs breakpoint id
    breakpoint_id(type_t t, num_t n):
        type_{t}, num_{n} {}

    /// Returns breakpoint type
    auto type() const { return type_; }

    /// Returns breakpoint number
    auto num() const { return num_; }

private:
    type_t type_;
    num_t num_;
};


/// Returns true if breakpoint ID is less than other breakpoint ID
inline bool operator<(const breakpoint_id & id1, const breakpoint_id & id2) {
    if (id1.type() == breakpoint_id::type_t::code &&
        id2.type() == breakpoint_id::type_t::watch) {

        return true;
    }

    if (id1.type() == id2.type() && id1.num() < id2.num()) {
        return true;
    }

    return false;
}


/// Returns true if two breakpoint IDs are equal
inline bool operator==(const breakpoint_id & id1, const breakpoint_id & id2) {
    return id1.type() == id2.type() && id1.num() == id2.num();
}


/// Returns true if two breakpoint IDs are not equal
inline bool operator!=(const breakpoint_id & id1, const breakpoint_id & id2) {
    return !(id1 == id2);
}


}
