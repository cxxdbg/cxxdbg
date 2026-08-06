// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file platform_group.cpp
/// Contains implementation of the platform_group class.

#include "platform_group.hpp"
#include "exec_command.hpp"
#include "po_command.hpp"


namespace po = boost::program_options;


namespace cxxdbg::dbg::cli {


platform_group::platform_group(exec_processor & exec_proc, platform_processor & p_proc):
command_group{"A set of commands for operating on a platform"} {
    reg_select(p_proc);
    reg_connect(p_proc);
    reg_disconnect(p_proc);

    // exec commands
    reg_exec_cmd(exec_proc, "file", "Commands to access files on the current platform.");
    reg_exec_cmd(exec_proc, "get-file", "Transfer a file from the remote end to the local host.");
    reg_exec_cmd(exec_proc, "get-size", "Get the file size from the remote end.");
    reg_exec_cmd(exec_proc, "list", "List all platforms that are available.");
    reg_exec_cmd(exec_proc, "mkdir", "Make a new directory on the remote end.");
    reg_exec_cmd(exec_proc, "process", "Commands to query, launch and attach to processes on the current platform.");
    reg_exec_cmd(exec_proc, "put-file", "Transfer a file from this system to the remote end.");
    reg_exec_cmd(exec_proc, "settings", "Set settings for the current target's platform, or for a platform by name.");
    reg_exec_cmd(exec_proc, "shell", "Run a shell command on the current platform.  Expects 'raw' input (see 'help raw-input'.)");
    reg_exec_cmd(exec_proc, "status", "Display status for the current platform.");
    reg_exec_cmd(exec_proc, "target-install", "Install a target (bundle or executable file) to the remote end.");
}


void platform_group::reg_exec_cmd(exec_processor & app_proc, const std::string & name, const std::string & desc) {
    reg_cmd(name, exec_command::make(app_proc, std::string("platform ") + name, desc));
}


void platform_group::reg_select(platform_processor & proc) {
    auto handler = [&proc](const po::variables_map & vars, const auto & c_handler) {
        if (vars.count("platform") == 0) {
            c_handler("error: platform name is not specified");
            return;
        }

        auto p_name = vars["platform"].as<std::string>();
        if (proc.select_platform(p_name)) {
            c_handler(std::string{"selected platform: "} + p_name);
        } else {
            c_handler(std::string{"error: platform not found: "} + p_name);
        }
    };

    auto cmd = make_po_command(handler);
    cmd->set_desc_help("Create a platform if needed and select it as the current platform.");
    cmd->opt_desc().add_options()("platform,p", po::value<std::string>(), "Platform name");
    cmd->popt_desc().add("platform", 1);
    reg_cmd("select", cmd);
}


void platform_group::reg_connect(platform_processor & proc) {
    auto handler = [&proc](const po::variables_map & vars, const auto & c_handler) {
        if (vars.count("url") == 0) {
            c_handler("error: url is not specified");
            return;
        }

        auto url = vars["url"].as<std::string>();
        auto local_cache_dir = vars["local-cache-dir"].as<std::string>();
        auto enable_rsync = vars.count("rsync") != 0;
        auto rsync_prefix = vars["rsync-prefix"].as<std::string>();
        auto rsync_opts = vars["rsync-opts"].as<std::string>();
        auto ignore_remote_hostname = vars.count("ignore-remote-hostname") != 0;

        proc.connect_to_platform(url, local_cache_dir, enable_rsync, rsync_opts, rsync_prefix, ignore_remote_hostname,
        [url, c_handler](auto && res) {
            if (res.is_ok()) {
                std::ostringstream msg;
                msg << "connected to platform " << res.value() << " with URL " << url;
                c_handler(msg.str());
            } else {
                std::ostringstream msg;
                msg << "can't connect to platform: " << res.error();
                c_handler(msg.str());
            }
        });
    };

    auto cmd = make_po_command(handler);
    cmd->set_desc_help("Connect to platform by providing a connection URL.");
    cmd->opt_desc().add_options()
        ("url,u", po::value<std::string>(), "Connect URL")
        ("rsync-prefix,P", po::value<std::string>()->default_value({}), "Platform-specific rsync prefix put before the remote path.")
        ("rsync-opts,R", po::value<std::string>()->default_value({}), "Platform-specific options required for rsync to work.")
        ("local-cache-dir,c", po::value<std::string>()->default_value({}), "Path in which to store local copies of files.")
        ("ignore-remote-hostname,i", "Do not automatically fill in the remote hostname when composing the rsync command.")
        ("rsync,r", "Enable rsync.");
    cmd->popt_desc().add("url", 1);
    reg_cmd("connect", cmd);
}


void platform_group::reg_disconnect(platform_processor & proc) {
    auto handler = [&proc](const po::variables_map & vars, const auto & c_handler) {
        proc.disconnect_from_platform();
        c_handler("disconnected");
    };

    auto cmd = make_po_command(handler);
    cmd->set_desc_help("Disconnect from the current platform.");
    reg_cmd("disconnect", cmd);
}


}
