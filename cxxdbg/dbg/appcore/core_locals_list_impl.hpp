// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file core_locals_list_impl.hpp
/// Contains definition of core_locals_list_impl class.

#pragma once

#include "core_watch_list_impl.hpp"


namespace cxxdbg::dbg::core {
    class target;
}


namespace cxxdbg::dbg::appcore {


/// \class core_locals_list_impl
/// Watch list implementation for list of locals in core library
class core_locals_list_impl: public core_watch_list_impl {
public:
    /// Constructor, makes locals watch list implementation with specified
    /// reference to async execution queue for core commands and
    /// reference to core target
    core_locals_list_impl(async::execution_queue & exec_queue, core::target & targ);

    /// Destructor, destroys object
    ~core_locals_list_impl() override = default;

private:
    /// Returns map of (id, value) in watch list
    std::map<watch_id, dbgfmt::named_fmt_result> get_variables() override;
};


}


