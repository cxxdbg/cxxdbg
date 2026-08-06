// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file stop_reason.cpp
/// Contains implementation of stop reason classes.

#include "stop_reason.hpp"
#include <sstream>
#include <string.h>


namespace cxxdbg::dbg {


stop_reason::~stop_reason() {
}


bool stop_reason::is_trace() const {
    return dynamic_cast<const stop_reason_trace*>(this) != nullptr;
}


bool stop_reason::is_break() const {
    return dynamic_cast<const stop_reason_breakpoint*>(this) != nullptr;
}


bool stop_reason::is_wpoint() const {
    return dynamic_cast<const stop_reason_watchpoint*>(this) != nullptr;;
}


bool stop_reason::is_signal() const {
    return dynamic_cast<const stop_reason_signal*>(this) != nullptr;
}


bool stop_reason::is_exec() const {
    return dynamic_cast<const stop_reason_exec*>(this) != nullptr;
}


bool stop_reason::is_crash() const {
    return dynamic_cast<const stop_reason_crash*>(this) != nullptr;
}


stop_reason_trace::~stop_reason_trace() {
}


stop_reason_breakpoint::~stop_reason_breakpoint() {
}


stop_reason_signal::stop_reason_signal(int s):
sig_num_{s} {
}


stop_reason_signal::~stop_reason_signal() {
}


int stop_reason_signal::sig_num() const {
    return sig_num_;
}


std::string stop_reason_signal::sig_name() const {
    static const std::string names[] = {
        "invalid signal",
        "SIGHUP",
        "SIGINT",
        "SIGQUIT",
        "SIGILL",
        "SIGTRAP",
        "SIGABRT",
        "SIGBUS",
        "SIGFPE",
        "SIGKILL",
        "SIGUSR1",
        "SIGSEGV",
        "SIGUSR2",
        "SIGPIPE",
        "SIGALRM",
        "SIGTERM",
        "SIGSTKFLT",
        "SIGCHLD",
        "SIGCONT",
        "SIGSTOP",
        "SIGTSTP",
        "SIGTTIN",
        "SIGTTOU",
        "SIGURG",
        "SIGXCPU",
        "SIGXFSZ",
        "SIGVTALRM",
        "SIGPROF",
        "SIGWINCH",
        "SIGIO",
        "SIGPWR",
        "SIGSYS"
    };

    if (sig_num() < sizeof(names) / sizeof(names[0])) {
        return names[sig_num()];
    }

    std::ostringstream sig_name;
    sig_name << "<unknown signal " << sig_num() << ">";
    return sig_name.str();
}


stop_reason_exec::~stop_reason_exec() {
}


stop_reason_crash::~stop_reason_crash() {
}


}
