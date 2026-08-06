// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file break_base_group.hpp
/// Contains definition of the break_base_group class.

#pragma once

#include "code_break_processor.hpp"
#include "command_group.hpp"
#include "exec_command.hpp"
#include "po_command.hpp"
#include "cxxdbg/dbg/core/breakpoint_id.hpp"
#include <boost/program_options.hpp>


namespace cxxdbg::dbg::cli {


/// Base class for breakpoints and watchpoints command groups.
/// Contains subcommands common to both groups
class break_base_group: public command_group {
    using variables_map = boost::program_options::variables_map;

    template <typename T>
    auto po_value() {
        return boost::program_options::value<T>();
    }

public:
    /// Constructs group with specified item type and name (breakpoint/watchpoint)
    break_base_group(breakpoint_type type,
                     const std::string & item_name,
                     exec_processor & exec_proc,
                     break_processor & break_proc);

private:
    /// Registers exec command
    void reg_exec(exec_processor & exec_proc,
                  const std::string & name,
                  const std::string & desc);

    /// Registers delete command
    void reg_delete(break_processor & proc);

    /// Registers enable command
    void reg_enable(break_processor & proc);

    /// Registers disable command
    void reg_disable(break_processor & proc);

    /// Registers modify command
    void reg_modify(break_processor & proc);

    /// Registers list command
    void reg_list(break_processor & proc);


    breakpoint_type item_type_;                         ///< Breakpoint type
    std::string item_name_;                             ///< List item name
    std::string item_name_mult_{item_name_ + "s"};      ///< List item name multiply

    /// List item name with first capital letter
    std::string item_name_cap_;
};


}


