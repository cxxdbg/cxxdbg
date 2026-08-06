// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file custom_watch_list_impl.hpp
/// Contains definition of the custom_watch_list_impl interface.

#pragma once

#include "watch_list_impl.hpp"


namespace cxxdbg::dbg {


/// Interface for custom watch list implementation
class custom_watch_list_impl: virtual public watch_list_impl {
public:
    /// Destructor, destroys object
    virtual ~custom_watch_list_impl() {}

    /// Adds expression to watch list. Calls handler with added node info after completion
    virtual void add_watch(const std::wstring & expr, const node_handler & handl) = 0;

    /// Remvoes expression with specified ID of root node. Calls handler after completion
    virtual void remove_watch(node_id id) = 0;

    /// Sets expression for watch with spefified ID of root node. Calls handler after completion
    virtual void set_watch(node_id id, const std::wstring & expr, const node_handler & handl) = 0;
};


}


