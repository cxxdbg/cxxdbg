// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file watch_processor.hpp
/// Contains definition of the watch_processor class.

#pragma once

#include "break_processor.hpp"
#include "cxxdbg/async/result.hpp"
#include <filesystem>


namespace cxxdbg::dbg::cli {


/// Tag for watchpoint breakpoint type
struct watch_tag {};


/// Watchpoint creation properties
struct watch_create_properties: public break_properties {
    bool is_read;
    bool is_write;
    size_t size;

    watch_create_properties():
        is_read{false}, is_write{true}, size{0} {}
};


/// Abstract processor for commands from watchpoint group
class watch_processor: virtual public break_processor {
public:
    /// Type of completion handler for functions creating watchpoint
    using watch_handler = async::result_handler<core::breakpoint_num>;

    /// Adds watchpoint for variable with specified name
    virtual void add_var_watchpoint(const std::string & name,
                                    const watch_create_properties & props,
                                    const watch_handler & handl) = 0;

    /// Adds watchpoint for value pointed by address specified by expression
    virtual void add_expr_watchpoint(const std::string & expr,
                                     const watch_create_properties & props,
                                     const watch_handler & handl) = 0;
};


}


