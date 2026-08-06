// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file core_debugger_impl.cpp
/// Contains implementation of core_debugger_impl class.

#include "core_debugger_impl.hpp"
#include "core_target_impl.hpp"
#include "utils.hpp"

#include "cxxdbg/dbg/module_info.hpp"
#include "cxxdbg/dbg/platform.hpp"
#include "cxxdbg/dbg/target_impl.hpp"
#include "cxxdbg/dbg/core/target.hpp"


namespace cxxdbg::dbg::appcore {


core_debugger_impl::core_debugger_impl(async::event_queue & main_queue):
main_thread_queue_(main_queue),
core_async_queue_(core_thread_queue_, main_thread_queue_),
dbg_(core_thread_queue_) {

    // starting core thread
    std::thread cthrd([this]() { core_thread_func(); });
    std::swap(core_thread_, cthrd);

    // listening for debugger events
    dbg_.connect_event([this](const std::string & ev) {
        main_thread_queue_.post([this, ev] {
            event_sig_(ev);
        });
    });
}


core_debugger_impl::~core_debugger_impl() {
    // posting exit event to core thread
    core_thread_queue_.post(std::function<void()>());

    // waiting for core thread exit
    core_thread_.join();
}


void core_debugger_impl::connect_to_platform(const platform_connection_options & opts,
                                             const async::result_handler<> & handler) {
    assert(opts.plat != nullptr && "platform should not be null");

    core_async_queue_.add_command(
    [handler](auto && res) {
        if (handler) {
            handler(res);
        }
    },
    [this, opts]()  {
        // connecting to platform
        dbg_.connect_to_platform(opts.plat->name(),
                                 opts.url,
                                 opts.local_cache_dir,
                                 opts.enable_rsync,
                                 opts.rsync_opts,
                                 opts.rsync_prefix,
                                 opts.ignore_remote_host_name);

        // setting exec search path
        if (!opts.exec_search_paths.empty()) {
            bool first = true;
            std::ostringstream str;
            for (auto && path : opts.exec_search_paths) {
                if (!first) {
                    str << ' ';
                } else {
                    first = false;
                }

                str << '\"';
                str << path;
                str << '\"';
            }

            dbg_.set_settings_value("target.exec-search-paths", str.str());
        }
    });
}


void core_debugger_impl::exec_cmds(const std::vector<std::string> & cmds,
                                   const execute_result_handler & handler) {

    core_async_queue_.add_command(
        // result handler
        [handler](const async::result<std::string> & res) {
            std::string res_str = res.is_ok() ?
                        res.value() :
                        std::string("ERROR: ") + res.error();
            handler(res_str);
        }
        ,
        // command handler
        [this, cmds]() -> std::string {
            bool first = true;
            std::string res;
            for (auto && cmd : cmds) {
                if (!first) {
                    res += '\n';
                } else {
                    first = false;
                }

                res += dbg_.execute(cmd);
            }

            return res;
        }
    );
}


/// Returns source position for main function
static dbg::source_position_info get_main_pos(core::target & targ) {
    core::code_position pos = targ.find_func("main");
    if (!pos)
        return {};

    return extract_source_position_info(pos.get_src_pos());
}


void core_debugger_impl::load_target(const std::filesystem::path & exe_path,
                                     const load_target_handler & handler) {

    core_async_queue_.add_command(
    handler,
    [this, exe_path]()  {
        // loading core target
        std::shared_ptr<core::target> ctarget = dbg_.load_target(exe_path);

        auto mods = core_target_impl::read_all_modules(*ctarget);

        // creating target implementation
        std::shared_ptr<dbg::target_impl> impl(new core_target_impl(dbg_,
                                                                    main_thread_queue_,
                                                                    core_async_queue_,
                                                                    ctarget,
                                                                    *mods));

        // loading target modules

        return target_load_result(impl, mods, get_main_pos(*ctarget));
    });
}


void core_debugger_impl::attach(const std::variant<unsigned long, std::string> & targ,
                                const load_target_handler & handler) {
    core_async_queue_.add_command(
    handler,
    [this, targ]()  {
        // loading core target
        std::shared_ptr<core::target> ctarget;
        if (auto * ppid = std::get_if<unsigned long>(&targ)) {
            ctarget = dbg_.attach(*ppid);
        } else if (auto * pname = std::get_if<std::string>(&targ)) {
            ctarget = dbg_.attach(*pname);
        } else {
            assert(false && "invalid target spec");
        }

        // loading target modules
        auto mods = core_target_impl::read_all_modules(*ctarget);

        // creating target implementation
        std::shared_ptr<dbg::target_impl> impl(new core_target_impl(dbg_,
                                                                    main_thread_queue_,
                                                                    core_async_queue_,
                                                                    ctarget,
                                                                    *mods));

        return target_load_result(impl, mods, get_main_pos(*ctarget));
    });
}


signal_connection core_debugger_impl::connect_on_event(const event_handler & handler) {
    return event_sig_.connect(handler);
}


/// Sets settings value
void core_debugger_impl::set_settings_value(const std::string & name,
                                            const std::string & val,
                                            const async::result_handler<> & handler) {
    core_async_queue_.add_command(
    [handler](auto && res) {
        if (handler) {
            handler(res);
        }
    },
    [this, name, val] {
        dbg_.set_settings_value(name, val);
    });
}


void core_debugger_impl::core_thread_func() {
    core_thread_queue_.run();
}


}
