// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file debugger_base.cpp
/// Contains implementation of debugger_base class.

#include "debugger_base.hpp"
#include "function_name_parser.hpp"
#include "log.hpp"
#include "target_base.hpp"
#include "cxxdbg/async/event_queue.hpp"

#include <lldb/API/SBBreakpoint.h>
#include <lldb/API/SBBreakpointLocation.h>
#include <lldb/API/SBBroadcaster.h>
#include <lldb/API/SBCommandInterpreter.h>
#include <lldb/API/SBCommandReturnObject.h>
#include <lldb/API/SBError.h>
#include <lldb/API/SBEvent.h>
#include <lldb/API/SBListener.h>
#include <lldb/API/SBProcess.h>
#include <lldb/API/SBStream.h>
#include <lldb/API/SBTarget.h>
#include <lldb/API/SBThread.h>
#include <lldb/Core/Mangled.h>

#include <boost/dll/runtime_symbol_info.hpp>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>


namespace fs = std::filesystem;


namespace cxxdbg::dbg::core {


namespace {


/// internal debugger event types
enum debugger_lldb_event {
    debugger_lldb_event_async_command = 1
};


}

/// True if lldb was initialized
static bool initialized = false;


/// Checks for LLDB error and throws exception
static void check_lldb_error(const lldb::SBError & err) {
    if (err.Success())
        return;

    std::ostringstream msg;
    msg << "Debugger error: " << err.GetCString();
    throw std::runtime_error(msg.str());
}


debugger_base::debugger_base(async::event_queue & eq):
main_queue_(eq) {
    // initialize lldb if not initialized
    if (!initialized) {
#if defined(__linux__)
        // setting up LLDB_DEBUGSERVER_PATH var to point to lldb-server path
        fs::path exe_dir{boost::dll::program_location().string()};
        exe_dir.remove_filename();
        fs::path lldb_server_path = exe_dir / ".." / "libexec" / "cxxdbg" / "lldb-server";
        setenv("LLDB_DEBUGSERVER_PATH", lldb_server_path.c_str(), 1);
#endif

        lldb::SBDebugger::Initialize();
        // TODO: do we need this flag after switching to direct memory read?
        //lldb_private::ValueObject::SetOmitEmptyBaseClasses(false);
        initialized = true;
    }

    // creating lldb instance
    dbg_ = lldb::SBDebugger::Create();

    // creating event queue for debugger listener
    lldb_event_queue_ = std::make_unique<lldb_listener_event_queue>(dbg_.GetListener());

    // setting up LLDB event dispatcher to main thread
    lldb_event_queue_->lldb_event_signal().connect([this](auto && event) {
        lldb::SBStream str;
        event.GetDescription(str);
        CXXDBG_LOG_SCAT_DEBUG(core, debugger) << "received LLDB event: " << std::string_view{str.GetData(), str.GetSize()};

        // This code is called in event thread.
        // Dispatching event to main queue
        this->main_queue_.post([this, event] {
            this->on_event(event);
        });
    });

    // TODO: restore that after full integration with debugger terminal
    // // start listening for interpreter quit command
    // dbg_.GetListener().StartListeningForEvents(dbg_.GetCommandInterpreter().GetBroadcaster(),
    //                                           lldb::SBCommandInterpreter::eBroadcastBitQuitCommandReceived);

    // telling lldb we want listen modules changed events
    const char * targ_bc_name = lldb::SBTarget::GetBroadcasterClassName();
    uint32_t events = lldb::SBTarget::eBroadcastBitModulesLoaded |
                      lldb::SBTarget::eBroadcastBitModulesUnloaded;
    dbg_.GetListener().StartListeningForEventClass(dbg_, targ_bc_name, events);

    // starting event thread
    std::thread thrd([this] { event_thread_func(); });
    std::swap(event_thread_, thrd);
}


debugger_base::~debugger_base() {
    // stopping event thread
    lldb_event_queue_->post_quit();
    event_thread_.join();
}


std::string debugger_base::execute(const std::string & cmd) {
    lldb::SBCommandReturnObject res;
    dbg_.GetCommandInterpreter().HandleCommand(cmd.c_str(), res);

    switch(res.GetStatus()) {
    case lldb::eReturnStatusSuccessFinishNoResult:
    case lldb::eReturnStatusSuccessFinishResult:
        return std::string(res.GetOutput(), res.GetOutputSize());

    case lldb::eReturnStatusFailed:
        return std::string(res.GetError(), res.GetErrorSize());

    case lldb::eReturnStatusQuit:
        return "<quit>";

    case lldb::eReturnStatusSuccessContinuingNoResult:
        return "lldb::eReturnStatusSuccessContinuingNoResult";

    case lldb::eReturnStatusSuccessContinuingResult:
        return "lldb::eReturnStatusSuccessContinuingResult";

    case lldb::eReturnStatusStarted:
        return std::string(res.GetError(), res.GetErrorSize());

    case lldb::eReturnStatusInvalid:
        return "lldb::eReturnStatusInvalid";

    default:
        assert(false && "Unhandled result status");
        return "";
    }
}


lldb::SBListener debugger_base::lldb_listener() {
    return dbg_.GetListener();
}


void debugger_base::set_settings_value(const std::string & name, const std::string & val) {
    lldb::SBDebugger::SetInternalVariable(name.c_str(), val.c_str(), dbg_.GetInstanceName());
}


void debugger_base::connect_to_platform(std::string_view name,
                                        const std::string & url,
                                        const std::string & local_cache_dir,
                                        bool enable_rsync,
                                        const std::string & rsync_opts,
                                        const std::string & rsync_prefix,
                                        bool ignore_remote_host_name) {
    // creating platform with specified name
    lldb::SBPlatform platform{std::string{name}.c_str()};

    // checking that platform was found
    if (!platform.IsValid()) {
        std::ostringstream msg;
        msg << "can't find platform with name '" << name << "'";
        throw std::runtime_error{msg.str()};
    }

    // selecting platform
    dbg_.SetSelectedPlatform(platform);

    // building connection options

    lldb::SBPlatformConnectOptions opts{url.c_str()};
    if (!local_cache_dir.empty()) {
        opts.SetLocalCacheDirectory(local_cache_dir.c_str());
    }

    if (enable_rsync) {
        opts.EnableRsync(rsync_opts.c_str(), rsync_prefix.c_str(), ignore_remote_host_name);
    }

    // connecting platform
    auto err = platform.ConnectRemote(opts);
    if (err.Success()) {
        return;
    }

    std::ostringstream msg;
    auto err_str = err.GetCString();
    msg << "can't connect to platform '" << name << "' with URL '" << url << ": "
        << (err_str != nullptr ? err_str : "unknown error");
    throw std::runtime_error{msg.str()};
}


boost::signals2::connection debugger_base::connect_lldb_event(const lldb_event_handler & handl) {
    return lldb_event_signal_.connect(handl);
}


boost::signals2::connection debugger_base::connect_event(const event_handler & handl) {
    return connect_lldb_event([handl](const lldb::SBEvent & event) {
        lldb::SBStream str;
        event.GetDescription(str);
        handl(std::string(str.GetData(), str.GetSize()));
    });
}


void debugger_base::post_lldb_event(const std::function<void ()> & handler) {
    lldb_event_queue_->post([this, handler] {
        // executed in event loop thread, posting handler to main thread
        main_queue_.post(handler);
    });
}


void debugger_base::event_thread_func() {
    // processing LLDB event queue
    lldb_event_queue_->run();
}


void debugger_base::on_event(lldb::SBEvent ev) {
    lldb_event_signal_(ev);
}


lldb::SBTarget debugger_base::do_load_target(const std::filesystem::path & exe_path) {

    // use absolute path for executable name
    fs::path abs_exe_path = exe_path;
    if (fs::exists(abs_exe_path)) {
        abs_exe_path = fs::canonical(exe_path);
    }

    lldb::SBError err;
    lldb::SBTarget targ = dbg_.CreateTarget(abs_exe_path.string().c_str(), nullptr, nullptr, true, err);

    // checking error
    check_lldb_error(err);

    return targ;
}


lldb::SBTarget debugger_base::do_attach(unsigned long pid) {

    // creating empty target
    lldb::SBTarget targ = dbg_.CreateTarget(nullptr);
    assert(targ.IsValid() && "Can't create LLDB empty target");

    // ataching to process
    lldb::SBListener listener;
    lldb::SBError err;
    lldb::SBProcess proc = targ.AttachToProcessWithID(listener, static_cast<lldb::pid_t>(pid), err);

    // checking error
    check_lldb_error(err);

    // checkint that process is valid and is in stopped state
    assert(proc.IsValid() && "LLDB attach returned invalid process");
    assert((proc.GetState() == lldb::eStateAttaching ||
            proc.GetState() == lldb::eStateStopped) &&
           "Invalid LLDB process state after attaching");

    return targ;
}


lldb::SBTarget debugger_base::do_attach(const std::string & name) {

    // creating empty target
    lldb::SBTarget targ = dbg_.CreateTarget(nullptr);
    assert(targ.IsValid() && "Can't create LLDB empty target");

    // ataching to process
    lldb::SBListener listener;
    lldb::SBError err;
    lldb::SBProcess proc = targ.AttachToProcessWithName(listener, name.c_str(), false, err);

    // checking error
    check_lldb_error(err);

    // checkint that process is valid and is in stopped state
    assert(proc.IsValid() && "LLDB attach returned invalid process");
    assert((proc.GetState() == lldb::eStateAttaching ||
            proc.GetState() == lldb::eStateStopped) &&
           "Invalid LLDB process state after attaching");

    return targ;
}


lldb::SBTarget debugger_base::do_get_dummy_target() {
    return dbg_.GetDummyTarget();
}


}
