// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file debugger_impl.hpp
/// Contains definition of debugger_impl class.

#pragma once

#include "forward.hpp"
#include "source_position_info.hpp"
#include "cxxdbg/app/signals.hpp"
#include "cxxdbg/async/async.hpp"
#include <filesystem>
#include <string>
#include <variant>


namespace cxxdbg::dbg {

class platform;
class platform_connection_options;


/// \class debugger_impl
/// Represents abstract debugger implementation, contains functions for
/// async loading targets and executing commands
class debugger_impl {
public:
    /// Result of loading target
    class target_load_result {
        typedef std::vector<module_info> module_info_vector;
        typedef std::shared_ptr<module_info_vector> module_info_vector_sp;

    public:
        target_load_result(const target_impl_sp & ti = target_impl_sp(),
                           const module_info_vector_sp & mds = module_info_vector_sp(),
                           const source_position_info & mpos = {}):
            targ_impl(ti), modules(mds), main_pos(mpos) {}

        target_impl_sp targ_impl;
        module_info_vector_sp modules;
        source_position_info main_pos;
    };


    /// Type of execute command result handler function
    typedef void execute_result_handler_func(const std::string &);

    /// Type of execute command result handler
    typedef std::function<execute_result_handler_func> execute_result_handler;

    /// Type of load target handler function
    typedef void load_target_handler_func(const async::result<target_load_result> &);

    /// Type of load target handler
    typedef std::function<load_target_handler_func> load_target_handler;

    /// Type of event handler function
    typedef void event_handler_func(const std::string &);

    /// Type of event handler
    typedef std::function<event_handler_func> event_handler;

    /// Connect to platform handler
    using connect_to_platform_handler = async::result_handler<>;


    /// Destructor, destroys object
    virtual ~debugger_impl() {}

    /// Connects debugger to platform
    virtual void connect_to_platform(const platform_connection_options & opts,
                                     const connect_to_platform_handler & handler = {}) = 0;

    /// Executes commands
    virtual void exec_cmds(const std::vector<std::string> & cmds,
                           const execute_result_handler & handler) = 0;

    /// Loads target with specified path to executable
    virtual void load_target(const std::filesystem::path & exe_path,
                             const load_target_handler & handler) = 0;

    /// Attaches to target with specified process ID
    virtual void attach(const std::variant<unsigned long, std::string> & targ,
                        const load_target_handler & handler) = 0;

    /// Connects to event signal
    virtual signal_connection connect_on_event(const event_handler & handler) = 0;

    /// Sets settings value
    virtual void set_settings_value(const std::string & name,
                                    const std::string & val,
                                    const async::result_handler<> & handler) = 0;
};


}


