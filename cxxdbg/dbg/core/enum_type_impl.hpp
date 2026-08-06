// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file enum_type_impl.hpp
/// Contains definition of the enum_type_impl class.

#pragma once

#include "type_impl.hpp"
#include "dbgfmt/backend/enum_type_impl.hpp"


namespace cxxdbg::dbg::core {


/// Represents implementation of enum type
class enum_type_impl: public type_impl, public dbgfmt::backend::enum_type_impl {
public:
    /// Constructor, makes enum value with specified pointer to lldb value
    enum_type_impl(target_base & targ, const lldb::SBType & t);

    /// Returns number of enum items
    std::size_t items_size() const override;

    /// Returns name of enum item with specified index
    std::string item_name(std::size_t index) const override;

    /// Returns value of enum item with specified index as signed integer
    std::int64_t item_value(std::size_t index) const override;
};


}
