// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file client_term_debug_child_process.hpp
/// Contains definition of the client_term_debug_child_process class.

#pragma once

#include "debug_child_process.hpp"
#include "client_term_process_launcher.hpp"
#include "cxxdbg/exec_stub/exec_stub_server.hpp"


namespace cxxdbg::dbg::dapsrv {


/// Child process launched/attached for debugging with CXXDBG in client terminal
class client_term_debug_child_process: virtual public debug_child_process {
public:
    /// Constructs child process with specified asio executor for executing code in work thread,
    /// and process launcher for launching child
    client_term_debug_child_process(const boost::asio::executor & exec,
                                    std::unique_ptr<client_term_process_launcher> && plauncher):
        debug_child_process{exec}, proc_launcher_{std::move(plauncher)} {}
 
    /// Prepares child proces to be launched with specified path to executable and arguments
    void prepare_launch(const path_t & exe_path,
                        const path_t & work_dir,
                        const std::vector<std::wstring> & args,
                        const std::map<std::wstring, std::wstring> & env,
                        const json_t & json_parameters,
                        const source_breakpoints_map & source_breakpoints,
                        const function_breakpoints_vector & function_breakpoints,
                        const instruction_breakpoints_vector & instruction_breakpoints,
                        const std::vector<std::wstring> & exception_breakpoint_filters) override;

    /// Processes configuration done request. Launches process for debugging or resumes
    /// attached process
    void process_configuration_done(const std::function<void (const std::string &)> & comp_handler) override;

private:
    path_t exe_path_;                   ///< Path to executable
    path_t work_dir_;                   ///< Working directory
    std::vector<std::wstring> args_;    ///< Launch arguments
    std::map<std::wstring, std::wstring> env_;                      ///< Launch environment
    std::unique_ptr<client_term_process_launcher> proc_launcher_;   ///< Process launcher for launching child
    std::unique_ptr<exec_stub::exec_stub_server> exec_stub_srv_;    ///< Exec stub server
};

}
