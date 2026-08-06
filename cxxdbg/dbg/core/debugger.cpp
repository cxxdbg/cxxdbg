// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file deubgger.cpp
/// Contains implementation of the debugger class.

#include "debugger.hpp"
#include "lldb_debugger_ext.hpp"
#include "target.hpp"


namespace cxxdbg::dbg::core {


bool debugger::initialized_ = false;


debugger::debugger(async::event_queue & eq):
debugger_base(eq) {
    if (!initialized_) {
        init_lldb_extensions();
        initialized_ = true;
    }

    init_lldb_extensions_instance(dbg(), false);
}


std::shared_ptr<target> debugger::load_target(const std::filesystem::path & exe_path) {
    return std::make_shared<target>(*this, exe_path);
}


std::shared_ptr<target> debugger::attach(unsigned long pid) {
    return std::make_shared<target>(*this, pid);
}


std::shared_ptr<target> debugger::attach(const std::string & name) {
    return std::make_shared<target>(*this, name);
}


target & debugger::dummy_target() {
    if (!dummy_targ_) {
        dummy_targ_ = std::shared_ptr<target>(new target{*this});
    }

    return *dummy_targ_;
}


}
