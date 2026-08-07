// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file main.cpp
/// Main file for cxxdbg-cli console application

#include "cxxdbg/app/version.hpp"
#include "cxxdbg/dbg/core/lldb_debugger_ext.hpp"
#include "cxxdbg/log/log.hpp"
#include "cxxdbg/log/log_init.hpp"
#include "cxxdbg/util/convert.hpp"
#include <lldb/API/SBDebugger.h>
#include <iostream>
#include <sstream>


using lldb_driver_version_hook_type = std::string (*)();
using lldb_driver_help_hook_type = std::string (*)();
using lldb_driver_init_hook_type = std::string (*)(const lldb::SBDebugger & debugger);

void set_lldb_driver_version_hook(lldb_driver_version_hook_type h);
void set_lldb_driver_help_hook(lldb_driver_help_hook_type h);
void set_lldb_driver_init_hook(lldb_driver_init_hook_type h);


static std::string cxxdbg_driver_version_hook() {
    std::ostringstream str;
    str << "cxxdbg version " << cxxdbg::version_display_str() << "\n";
    return str.str();
}


static std::string cxxdbg_driver_help_hook() {
    std::ostringstream str;
    str << cxxdbg_driver_version_hook() << "\n";
    return str.str();
}


static std::string cxxdbg_driver_init_hook(const lldb::SBDebugger & dbg) {
    std::ostringstream output;

    // initializing log
#ifdef _WIN32
    auto home_dir_var_name = "USERPROFILE";
#else
    auto home_dir_var_name = "HOME";
#endif
    auto log_file_path = std::filesystem::path(getenv(home_dir_var_name)) / ".cxxdbg/cxxdbg.log";
    ::cxxdbg::log::init(false, log_file_path);

    // initializing cxxdbg extensions in debugger instance
    ::cxxdbg::dbg::core::init_lldb_extensions_instance(dbg, true);

    return output.str();
}


int main(int argc, char const * argv[]) {
    // initializing cxxdbg extensions in lldb
    ::cxxdbg::dbg::core::init_lldb_extensions();

    // setting hooks for LLDB driver
    set_lldb_driver_init_hook(&cxxdbg_driver_init_hook);
    set_lldb_driver_help_hook(&cxxdbg_driver_help_hook);
    set_lldb_driver_version_hook(&cxxdbg_driver_version_hook);

    // invoking main function from LLDB driver tool library
    int lldb_driver_main(int argc, char const *argv[]);
    return lldb_driver_main(argc, argv);
}
