// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file bp_io_launcher.cpp
/// Contains implementation of the bp_io_launcher class.

#include "bp_io_launcher.hpp"
#include "bp_utils.hpp"
#include "log.hpp"
#include "cxxdbg/proc/async_launcher.hpp"
#include <codecvt>
#include <boost/asio/post.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include "cxxdbg/boost_process/process.hpp"


namespace ba = boost::asio;
namespace bp = boost::process;
namespace fs = std::filesystem;


namespace cxxdbg::proc {



bp_io_launcher::stdio_bp_child_monitor::stdio_bp_child_monitor(boost::asio::io_context & io_c,
                                                               const boost::asio::executor & e_exec,
                                                               bp::child && bp_child,
                                                               pipe_ptr && stdout_p,
                                                               pipe_ptr && stderr_p,
                                                               pipe_ptr &&stdin_p):
base_type{io_c, e_exec, std::move(bp_child), *stdin_p, *stdout_p},
std_out_pipe_{std::move(stdout_p)},
std_err_pipe_{std::move(stderr_p)},
std_in_pipe_{std::move(stdin_p)},
std_err_buffer_(4096) {
}


bp_io_launcher::stdio_bp_child_monitor::~stdio_bp_child_monitor() {
    // checking that buffers were cleared (that indicates that process exit was handled)
    assert(all_buffers_cleared() && "process is not exited");
}


bool bp_io_launcher::stdio_bp_child_monitor::all_buffers_cleared() const {
    return base_type::all_buffers_cleared() && std_err_buffer_.empty();
}


void bp_io_launcher::stdio_bp_child_monitor::start_read_stderr() {
    // executed in pool thread or in main thread on first call

    CXXDBG_PROC_LOG_TRACE << "child " << this << " start read stderr";

    std_err_pipe_->async_read_some(ba::buffer(std_err_buffer_),
                                   [this](auto && ecode, auto && size) {
        // executed in pool thread

        CXXDBG_PROC_LOG_TRACE << "child " << this << " read stderr complete: "
                            << ecode.value() << ", " << size;

        if (size != 0) {
            // sending notification in main queue
            std::string str(&std_err_buffer_[0], size);
            ba::post(event_exec(), [this, str] {
                // executed in main thread
                stderr_received(str);
            });
        }

        if (ecode.value() != 0 || size == 0) {
            // Process exited, clearing stderr buffer to indicate it's not used anymore
            std_err_buffer_.clear();

            // sending exit signal if all buffers were cleared
            if (all_buffers_cleared()) {
                send_exit_signal();
            }
        } else {
            // reading stderr again
            start_read_stderr();
        }
    });
}


bp_io_launcher::bp_io_launcher(boost::asio::io_context & ioc, boost::asio::executor event_exec):
io_ctx_{ioc},
event_exec_{event_exec} {
}


bp_io_launcher::~bp_io_launcher() {
}


std::unique_ptr<child_io_monitor> bp_io_launcher::launch_io(const launch_parameters & pars) {
    CXXDBG_PROC_LOG_INFO << "launch io: " << convert_launch_pars_to_string(pars);

    // creating pipes for reading process stdout and stderr
    auto std_out_pipe = std::make_unique<bp::async_pipe>(io_ctx_);
    auto std_err_pipe = std::make_unique<bp::async_pipe>(io_ctx_);
    auto std_in_pipe = std::make_unique<bp::async_pipe>(io_ctx_);

    // launching process
    auto bp_child = create_bp_child(pars,
                                    bp::std_in < *std_in_pipe,
                                    bp::std_out > *std_out_pipe,
                                    bp::std_err > *std_err_pipe);

    // creating child object
    auto chld = std::make_unique<stdio_bp_child_monitor>(io_ctx_,
                                                         event_exec_,
                                                         std::move(bp_child),
                                                         std::move(std_out_pipe),
                                                         std::move(std_err_pipe),
                                                         std::move(std_in_pipe));

    // starting reading of stdout and stderr from child
    chld->start_read_stdout();
    chld->start_read_stderr();

    return chld;
}


}
