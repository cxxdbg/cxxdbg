// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file break_processor.hpp
/// Contains definition of the break_processor class.

#pragma once

#include "break_processor.hpp"
#include "break_properties.hpp"
#include "cxxdbg/async/result.hpp"
#include <filesystem>


namespace cxxdbg::dbg::cli {


struct bp_tag {};


/// Abstract processor for commands from breakpoint group
class code_break_processor: virtual public break_processor {
    /// Type of breakpoint number
    using num_t = core::breakpoint_num;

public:
    /// Type of result handler for breakpoint functions
    typedef async::result_handler<num_t> code_break_handler;

    /// Adds breakpoint at function with specified name
    virtual void add_function_breakpoint(const std::string & name,
                                         const break_properties & props,
                                         const code_break_handler & handl) = 0;

    /// Adds breakpoint when exception thrown
    virtual void add_exception_thrown_breakpoint(const break_properties & props,
                                                 const code_break_handler & handl) = 0;

    /// Adds breakpoint when exception caught
    virtual void add_exception_caught_breakpoint(const break_properties & props,
                                                 const code_break_handler & handl) = 0;

    /// Adds breakpoint at specified source position
    virtual void add_srcpos_breakpoint(const std::filesystem::path & file,
                                       unsigned int line,
                                       const break_properties & props,
                                       const code_break_handler & handl) = 0;

    /// Adds breakpoint at specified address
    virtual void add_address_breakpoint(unsigned long addr,
                                        const break_properties & props,
                                        const code_break_handler & handl) = 0;
};


}


