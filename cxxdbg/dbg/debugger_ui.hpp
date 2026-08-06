// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file debugger_ui.hpp
/// Contains definition of the debugger_ui class.

#pragma once

#include <filesystem>
#include <string>


namespace cxxdbg::dbg {


/// Abstract class that represents implementation of debugger UI
class debugger_ui {
public:
    /// Virtual destructor
    virtual ~debugger_ui() = default;

    /// Notifies user about starting loading module
    virtual void notify_module_load_start(const std::filesystem::path & mod) = 0;

    /// Notifies user about finishing loading module
    virtual void notify_module_load_finish() = 0;

    /// Displays info message in application
    virtual void show_info_message(const std::wstring & msg) = 0;

    /// Asks user for enter step until line number
    virtual bool ask_step_until_line_number(unsigned int & line_number) = 0;
};



}
