// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file core_debugger_impl.hpp
/// Contains definition of core_debugger_impl class.

#pragma once

#include "cxxdbg/async/async_execution_queue.hpp"
#include "cxxdbg/async/simple_event_queue.hpp"
#include "cxxdbg/dbg/debugger_impl.hpp"
#include "cxxdbg/dbg/core/debugger.hpp"
#include "dbgfmt/context.hpp"


namespace cxxdbg::dbg::appcore {


/// \class core_debugger_impl
/// debugger_impl implementation for core library
class core_debugger_impl: public dbg::debugger_impl {
public:
    /// Constructor, makes debugger implementation with specified
    /// reference to main thread event queue
    core_debugger_impl(async::event_queue & main_queue);

    /// Destructor, destroys object
    ~core_debugger_impl() override;

    /// Connects debugger to platform
    void connect_to_platform(const platform_connection_options & opts,
                             const async::result_handler<> & handler = {}) override;

    /// Executes commands
    void exec_cmds(const std::vector<std::string> & cmd,
                   const execute_result_handler & handler) override;

    /// Loads target with specified path to executable
    void load_target(const std::filesystem::path & exe_path,
                     const load_target_handler & handler) override;

    /// Attaches to target with specified process name of ID
    void attach(const std::variant<unsigned long, std::string> & targ,
                const load_target_handler & handler) override;

    /// Connects to event signal
    signal_connection connect_on_event(const event_handler & handler) override;

    /// Sets settings value
    void set_settings_value(const std::string & name,
                            const std::string & val,
                            const async::result_handler<> & handler) override;

private:
    /// Main function for thread for executing core debugger functions
    void core_thread_func();

    async::event_queue & main_thread_queue_;        ///< Reference to main thread queue
    async::simple_event_queue core_thread_queue_;   ///< Queue for posting events to core thread
    async::execution_queue core_async_queue_; ///< Queue for executing async commands in core thread
    std::thread core_thread_;                       ///< Core debugger thread
    core::debugger dbg_;                            ///< Core debugger

    boost::signals2::signal<void (const std::string &)> event_sig_;   ///< Event signal
};


}


