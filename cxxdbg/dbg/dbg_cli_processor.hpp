// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file dbg_cli_processor.hpp
/// Contains definition of the dbg_cli_processor class.

#pragma once

#include "cli_processor.hpp"


namespace cxxdbg::dbg {


class application;


/// Implementation of command processor for debugger terminal in
/// standalone debugger application. Redirects all target management
/// calls to application object
class dbg_cli_processor: public cli_processor {
public:
    /// Constructs processor with specified reference to application object
    dbg_cli_processor(application & app);

    /// Virtual destructor
    ~dbg_cli_processor() override = default;

    /// Attaches to process with specified id
    void attach_pid(unsigned long pid, const process_handler & handler) override;

    /// Attaches to process with specified name
    void attach_name(const std::string & name, const process_handler & handler) override;

    /// Asks user to enter ID of process and attaches to id
    void attach(const process_handler & handler) override;

    /// Detaches from current process
    void detach(const process_handler & handler) override;

    /// Kills current process
    void kill(const process_handler & handler) override;

    /// Launches debugging of current target with specified launch flags
    void launch(const std::filesystem::path & work_dir,
                const std::vector<std::string> & cmd_args,
                bool save_default,
                const process_handler & handler) override;

    /// Loads target with specified file name and launch options
    void load_target(const std::filesystem::path & file_name,
                     const std::filesystem::path & work_dir,
                     const std::vector<std::string> & cmd_args,
                     const completion_handler & handler) override;

    /// Closes current target
    void close_target(const completion_handler & handler) override;

    /// Selects platform
    bool select_platform(std::string_view name) override;

    /// Connects to platform. Calls completion handler with selected platform name
    void connect_to_platform(const std::string_view url,
                             const std::string & local_cache_dir,
                             bool enable_rsync,
                             const std::string & rsync_opts,
                             const std::string & rsync_prefix,
                             bool ignore_remote_host_name,
                             const async::result_handler<std::string> & handler) override;

    /// Disconnects from platform
    void disconnect_from_platform() override;

private:
    application & app_;         ///< Reference to application instance
};


}


