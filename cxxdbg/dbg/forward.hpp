// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file forward.hpp
/// Contains forward declarations of cxxdbg application classes.

#pragma once

#include <memory>

namespace cxxdbg::dbg {


class application;
class breakpoint;
class code_breakpoint;
class code_breakpoint_info;
class breakpoint_location;
class breakpoint_location_info;
class code_position;
class command_line;
class debugger;
class debugger_impl;
class function_command_handler;
class launch_options;
class module_info;
class process_context;
class source_file;
class source_model;
class source_position_info;
class source_tree;
class source_tree;
class source_tree_builder_directory;
class source_position_breakpoint_info;
class stack_frame;
class target;
class target_impl;
class thread;
class thread_list;
class thread_list_info;
class watch_list;
class watch_list_impl;
class watch_list_tree_node;
class watchpoint;
class expand_state_memento;
class expand_state_manager;


typedef std::shared_ptr<debugger_impl> debugger_impl_sp;
typedef std::shared_ptr<function_command_handler> function_command_handler_sp;
typedef std::shared_ptr<target> target_sp;
typedef std::shared_ptr<target_impl> target_impl_sp;
typedef std::shared_ptr<watch_list> watch_list_sp;
typedef std::shared_ptr<watch_list_impl> watch_list_impl_sp;


}


