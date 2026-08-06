// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread_base.cpp
/// Contains implementation of thread_base class.

#include "thread_base.hpp"
#include "target_base.hpp"
#include <lldb/Target/Thread.h>
#include <chrono>
#include <iostream>


namespace cxxdbg::dbg::core {


const_thread::const_thread(target_base & targ, const lldb::SBThread & thrd):
targ_{targ}, thrd_(thrd) {
}


const_thread::const_thread(const const_thread &) = default;


const_thread::~const_thread() {
}


unsigned long const_thread::id() const {
    return static_cast<unsigned long>(thrd().GetThreadID());
}


std::size_t const_thread::selected_frame_index() const {
    return static_cast<std::size_t>(thrd().GetSelectedFrame().GetFrameID());
}


stop_reason_t const_thread::stop_reason() const {
    lldb::StopReason str = thrd().GetStopReason();
    switch (str) {
    case lldb::eStopReasonInvalid:
    case lldb::eStopReasonNone:
        return stop_reason_t::unknown;
    case lldb::eStopReasonTrace:
        return stop_reason_t::trace;
    case lldb::eStopReasonBreakpoint:
        return stop_reason_t::breakpoint;
    case lldb::eStopReasonWatchpoint:
        return stop_reason_t::watchpoint;
    case lldb::eStopReasonSignal:
    case lldb::eStopReasonException:
        return stop_reason_t::signal;
    case lldb::eStopReasonPlanComplete:
        return stop_reason_t::trace;
    case lldb::eStopReasonExec:
        return stop_reason_t::exec;
    default:
        CXXDBG_CORE_LOG_ERROR << "unknown stop reason: " << str;
        assert(false && "unknown stop reason");
        return stop_reason_t::unknown;
    }
}


lldb::SBThread const_thread::thrd() const {
    return thrd_;
}



thread_base::thread_base(target_base & targ, size_t idx):
const_thread(targ, targ.lldb_thread_at(idx)) {
}


thread_base::~thread_base() {
}


void thread_base::set_selected_frame(std::size_t i) const {
    thrd().SetSelectedFrame(static_cast<uint32_t>(i));
}


}
