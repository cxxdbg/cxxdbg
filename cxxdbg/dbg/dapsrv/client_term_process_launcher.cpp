// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file client_term_process_launcher.cpp
/// Contains implementation of the client_term_process_launcher class.

#include "client_term_process_launcher.hpp"
#include "server_impl.hpp"


namespace cxxdbg::dbg::dapsrv {


void client_term_process_launcher::launch(const std::filesystem::path & exe_path,
                                          const std::filesystem::path & work_dir,
                                          const std::vector<std::wstring> & args,
                                          const std::map<std::wstring, std::wstring> & env,
                                          const std::function<void (long, const std::string&)> & handler) {

    std::vector<std::wstring> args_with_exe = {exe_path.wstring()};
    std::copy(args.begin(), args.end(), std::back_inserter(args_with_exe));

    s_impl_.send_run_in_terminal(work_dir, args_with_exe, true, {}, env,
    [this, handler](long pid, long shell_pid, const std::string & err_msg) {
        handler(pid, err_msg);
    });
}


}
