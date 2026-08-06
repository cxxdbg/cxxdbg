// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file registers_list.hpp
/// Contains definition of the registers_list class.

#pragma once

#include "core_watch_list_impl.hpp"


namespace cxxdbg::dbg::appcore {


/// Watch list implementation for registers
class registers_list_impl: public core_watch_list_impl {
public:
    /// Constructors watch list implementation with specified
    /// reference to async execution queue for core commands and
    /// reference to core target
    registers_list_impl(async::execution_queue & exec_queue, core::target & targ):
        core_watch_list_impl{exec_queue, targ} {}

    /// Destructor, destroys object
    ~registers_list_impl() override = default;

private:
    /// Returns map of (id, value) in watch list
    std::map<watch_id, dbgfmt::named_fmt_result> get_variables() override;
};


}
