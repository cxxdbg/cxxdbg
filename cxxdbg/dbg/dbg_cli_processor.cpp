// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file dbg_cli_processor.cpp
/// Contains implementation of the dbg_cli_processor class.

#include "dbg_cli_processor.hpp"
#include "application.hpp"
#include "debugger.hpp"
#include "platform.hpp"
#include "target.hpp"


namespace cxxdbg::dbg {


dbg_cli_processor::dbg_cli_processor(application & app):
cli_processor(app.dbg()),
app_{app} {
}


void dbg_cli_processor::attach_pid(unsigned long pid, const process_handler & handler) {
    bool res = app_.ask_and_attach(pid, {});
    if (!res) {
        // canceled by user
        handler(async::result<unsigned long>{"canceled by user"});
        return;
    }

    handler(async::ok_result(pid));
}


void dbg_cli_processor::attach_name(const std::string & name, const process_handler & handler) {
    bool res = app_.ask_and_attach(name, {});
    if (!res) {
        // canceled by user
        handler(async::result<unsigned long>{"canceled by user"});
        return;
    }

    handler(async::ok_result(0));
}


void dbg_cli_processor::attach(const process_handler & handler) {
    unsigned long pid = app_.ask_pid_and_attach();
    if (pid == 0) {
        // canceled by user
        async::result<unsigned long> res("canceled by user");
        handler(res);
        return;
    }

    handler(async::ok_result(pid));
}


void dbg_cli_processor::detach(const process_handler & handler) {
    if (!app_.has_process()) {
        handler(async::result<unsigned long>("process is not launched"));
        return;
    }

    unsigned long pid = app_.current_target().pid();
    app_.detach();
    handler(async::ok_result(pid));
}


void dbg_cli_processor::kill(const process_handler & handler) {
    if (!app_.has_process()) {
        handler(async::result<unsigned long>("process is not launched"));
        return;
    }

    unsigned long pid = app_.current_target().pid();
    app_.terminate();
    handler(async::ok_result(pid));
}


void dbg_cli_processor::launch(const std::filesystem::path & work_dir,
                               const std::vector<std::string> & cmd_args,
                               bool save_default,
                               const process_handler & handler) {
    // checking that target is loaded
    if (!app_.has_target()) {
        handler(async::result<unsigned long>{"error: target is not loaded"});
        return;
    }

    // checking that target is not started
    if (app_.has_process()) {
        handler(async::result<unsigned long>{"error: target already started"});
        return;
    }

    // building launch options

    launch_options lopts = app_.def_launch_opts();

    if (!work_dir.empty())
        lopts.set_work_dir(work_dir);

    if (!cmd_args.empty())
        std::copy(cmd_args.begin(), cmd_args.end(), std::back_inserter(lopts.launch_args()));

    // launching

    unsigned long pid = app_.launch(lopts, save_default);

    // calling completion handler
    handler(async::ok_result(pid));
}


void dbg_cli_processor::load_target(const std::filesystem::path & file_name,
                                    const std::filesystem::path & work_dir,
                                    const std::vector<std::string> & cmd_args,
                                    const completion_handler & handler) {
    if (app_.state() != application::state_t::initial) {
        handler(async::result<>("target already loaded"));
        return;
    }

    app_.load_target(file_name, work_dir, cmd_args);
    handler(async::result<>{});
}


void dbg_cli_processor::close_target(const completion_handler & handler) {
    if (app_.state() == application::state_t::initial) {
        handler(async::result<>("target not loaded"));
        return;
    }

    if (app_.close()) {
        handler({});
    } else {
        handler(async::result<>{"close was canceled"});
    }
}


bool dbg_cli_processor::select_platform(std::string_view name) {
    // looking for platform with specified name
    for (auto && p : app_.dbg().platforms().all()) {
        if (p->name() == name) {
            app_.dbg().set_selected_platform(p);
            return true;
        }
    }

    return false;
}


void dbg_cli_processor::connect_to_platform(const std::string_view url,
                                            const std::string & local_cache_dir,
                                            bool enable_rsync,
                                            const std::string & rsync_opts,
                                            const std::string & rsync_prefix,
                                            bool ignore_remote_host_name,
                                            const async::result_handler<std::string> & handler) {
    platform_connection_options opts;
    opts.plat = app_.dbg().selected_platform();
    opts.url = url;
    opts.local_cache_dir = local_cache_dir;
    opts.enable_rsync = enable_rsync;
    opts.rsync_opts = rsync_opts;
    opts.rsync_prefix = rsync_prefix;
    opts.ignore_remote_host_name = ignore_remote_host_name;

    try {
        if (app_.ask_and_connect(opts)) {
            async::result<std::string> res{""};
            res.set_value(opts.plat->name());
            handler(res);
        } else {
            handler(async::result<std::string>{"operation cancelled by user"});
        }
    }
    catch (std::runtime_error & err) {
        async::result<std::string> res{err.what()};
        handler(res);
    }
}


void dbg_cli_processor::disconnect_from_platform() {
    app_.ask_and_disconnect();
}


}
