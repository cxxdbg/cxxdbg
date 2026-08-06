// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file client_term_process_launcher.hpp
/// Contains definition of the client_term_process_launcher class.

#pragma once

#include "server_impl.hpp"
#include <filesystem>
#include <functional>
#include <map>
#include <vector>


namespace cxxdbg::dbg::dapsrv {


class server_impl;


/// Process launcher that launches process in client rerminal via runInTerminal reverse request
class client_term_process_launcher {
public:
    /// Constructs launcher with specified reference to server implementation
    client_term_process_launcher(server_impl & simpl):
        s_impl_{simpl} {}

    /// Default destructor
    ~client_term_process_launcher() = default;

    /// Launches process with specified path to executable, work dir, and arguments.
    /// Calls completion handler after launch is complete
    void launch(const std::filesystem::path & exe_path,
                const std::filesystem::path & work_dir,
                const std::vector<std::wstring> & args,
                const std::map<std::wstring, std::wstring> & env,
                const std::function<void (long, const std::string&)> & handler);

private:
    server_impl & s_impl_;          ///< Reference to server implementation
};



}