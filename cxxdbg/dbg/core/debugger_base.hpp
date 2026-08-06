// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file debugger_base.hpp
/// Contains definition of debugger_base class.

#pragma once

#include "code_model.hpp"
#include "lldb_listener_event_queue.hpp"
#include "cxxdbg/async/async.hpp"

#include <lldb/API/SBBroadcaster.h>
#include <lldb/API/SBDebugger.h>
//#include <lldb/Core/Mangled.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <thread>
#include <vector>
#include <queue>
#include <boost/signals2/signal.hpp>


namespace cxxdbg::dbg::core {


class function_name_parser;
class target_base;


/// \class debugger_base
/// Base class for the debugger class. Contains logic dependent only from LLVM
class debugger_base {
    friend class target_base;

public:
    /// Type of LLDB event handler function
    typedef void lldb_event_func(const lldb::SBEvent&);

    /// Type of LLDB event handler
    typedef std::function<lldb_event_func> lldb_event_handler;

    /// Type of event handler function
    typedef void event_handler_func(const std::string &);

    /// Type of event handler
    typedef std::function<event_handler_func> event_handler;

    /// Type of scoped connection to signal
    typedef boost::signals2::scoped_connection scoped_connection;

    /// Constructs debugger with specified reference to event queue for posting
    /// events to main thread
    debugger_base(async::event_queue & eq);

    /// Destructor, destroys debugger
    virtual ~debugger_base();

    /// Executes command with specified text, returns command output
    std::string execute(const std::string & cmd);

    /// Returns lldb listener for debugger
    lldb::SBListener lldb_listener();

    /// Sets settings value. Throws exception on error
    void set_settings_value(const std::string & name, const std::string & val);

    /// Connects to platform with specified name, URL and options
    void connect_to_platform(std::string_view name,
                             const std::string & url,
                             const std::string & local_cache_dir = {},
                             bool enable_rsync = false,
                             const std::string & rsync_opts = {},
                             const std::string & rsync_prefix = {},
                             bool ignore_remote_host_name = false);

    /// Connects to LLDB event signal
    boost::signals2::connection connect_lldb_event(const lldb_event_handler & handl);

    /// Connects to event signal
    boost::signals2::connection connect_event(const event_handler & handl);

    /// Returns reference to code model implementation
    virtual const code_model & cm() const = 0;

    /// Returns shared pointer to LLDB debugger object
    auto & dbg() const { return dbg_; }

    /// Posts event to LLDB event queue. The event handler will be executed
    /// in the main thread (will be posted to main queue specified in constructor).
    /// This function is needed to be able to execute code after all pending LLDB events
    /// are processed and posted to main queue.
    void post_lldb_event(const std::function<void ()> & handler);

private:
    // noncopyable
    debugger_base(const debugger_base&) = delete;
    debugger_base & operator=(const debugger_base&) = delete;

    /// Event thread function
    void event_thread_func();

    /// Called when new LLDB event received
    void on_event(lldb::SBEvent ev);

    /// Loads target with specified exe path, returns LLDB target
    lldb::SBTarget do_load_target(const std::filesystem::path & exe_path);

    /// Attaches to target with specified PID, returns LLDB target
    lldb::SBTarget do_attach(unsigned long pid);

    /// Attaches to target with specified process name, returns LLDB target
    lldb::SBTarget do_attach(const std::string & name);

    /// Returns LLDB dummy target
    lldb::SBTarget do_get_dummy_target();

    async::event_queue & main_queue_;               ///< Main thread event queue
    lldb::SBDebugger dbg_;                          ///< lldb debugger

    /// LLDB event queue
    std::unique_ptr<lldb_listener_event_queue> lldb_event_queue_;

    std::thread event_thread_;                      ///< Event queue thread

    /// LLDB event signal
    boost::signals2::signal<lldb_event_func> lldb_event_signal_;
};


}
