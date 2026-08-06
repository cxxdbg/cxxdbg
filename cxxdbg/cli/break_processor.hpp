// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file break_processor.hpp
/// Contains definition of the break_base_processor class.

#pragma once

#include "break_properties.hpp"
#include "cxxdbg/async/result.hpp"
#include "cxxdbg/dbg/core/breakpoint_id.hpp"


namespace cxxdbg::dbg::cli {


/// Type of breakpoint type
using breakpoint_type = core::breakpoint_type;

/// Type of breakpoint number
using breakpoint_num = core::breakpoint_num;

/// Type of breakpoint ID
using breakpoint_id = core::breakpoint_id;


/// Base class for breakpoint and watchpoint processors
class break_processor {
protected:
    /// Type of breakpoint id
    using id_t = core::breakpoint_id;

    /// Type of breakpoint type
    using type_t = core::breakpoint_type;

public:
    /// Type of completion handler
    using bp_handler = async::result_handler<>;

    /// Deletes breakpoint with specified id
    virtual void delete_breakpoint(const id_t & id, const bp_handler & h) = 0;

    /// Enabled breakpoint with specified id
    virtual void enable_breakpoint(const id_t & id, const bp_handler & h) = 0;

    /// Disables breakpoint with specified id
    virtual void disable_breakpoint(const id_t & id, const bp_handler & h) = 0;

    /// Sets breakpoint properties
    virtual void set_breakpoint_props(const id_t & id,
                                      const break_properties & props,
                                      const bp_handler & res) = 0;

    /// Prints list of breakpoints of specified type and returns it as string
    virtual std::string list_breakpoints(type_t type) = 0;
};


}


