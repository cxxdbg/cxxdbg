// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file platform.cpp
/// Contains implementation of platform description classes.

#include "platform.hpp"
#include <ranges.hpp>


namespace cxxdbg::dbg {


platform_list::platform_list() {
    register_platform("host", "Local host platform.");
    register_platform("remote-freebsd", "Remote FreeBSD user platform plug-in.");
    register_platform("remote-linux", "Remote Linux user platform plug-in.");
    register_platform("remote-netbsd", "Remote NetBSD user platform plug-in.");
    register_platform("remote-openbsd", "Remote OpenBSD user platform plug-in.");
    register_platform("remote-windows", "Remote Windows user platform plug-in.");
    register_platform("remote-ios", "Remote iOS platform plug-in.");
    register_platform("remote-macosx", "Remote Mac OS X user platform plug-in.");
    register_platform("remote-gdb-server", "A platform that uses the GDB remote protocol as the communication transport.");
}



const platform * platform_list::host() const {
    auto a = all();
    assert(!std::ranges::empty(a) && "platform list is empty");
    return *std::ranges::begin(a);
}


void platform_list::register_platform(std::string name, std::string desc) {
    platforms_.push_back(std::make_unique<platform>(std::move(name), std::move(desc)));
}


}
