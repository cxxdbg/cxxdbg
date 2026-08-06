// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file bp_io_child_monitor.hpp
/// Contains definition of the basic_bp_io_child_monitor template class.

#pragma once

#include "child_io_monitor.hpp"
#include "log.hpp"
#include <boost/asio/executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/write.hpp>
#include "cxxdbg/boost_process/process.hpp"


namespace cxxdbg::proc {


/// Implementation of IO child monitor via boost.process and boost.asio
/// that uses generic Sink and Source to read/write stdio.
template <typename StdinSinkType, typename StdoutSourceType>
class basic_bp_io_child_monitor: public child_io_monitor {
public:
    /// Constructs child object with specified reference to IO context,
    /// executor for posting events, boost process child,
    /// and references to stdin source and stdout sink
    basic_bp_io_child_monitor(boost::asio::io_context & io_c,
                              const boost::asio::executor & e_exec,
                              boost::process::child && bp_child,
                              StdinSinkType & in,
                              StdoutSourceType & out):
    io_ctx_{io_c},
    event_exec_{e_exec},
    bp_child_(std::move(bp_child)),
    stdin_sink_{in},
    stdout_source_{out},
    stdout_buffer_(4096) {
    }

    /// Destroys child object and checks exited flag
    ~basic_bp_io_child_monitor() override {
        assert(exited_ && "process is not exited");
    }

    /// Starts process termination
    void terminate() override {
        bp_child_.terminate();
    }

    /// Sends stdin to child process
    void send_stdin(const std::string & data) override {
        // executed in main thread

        // ignoring empty data (this indicates closing stdin)
        if (data.empty()) {
            return;
        }

        do_send_stdin(data);
    }

    /// Closes stdin of child process
    void close_stdin() override {
        // executed in main thread

        // sending empty buffer. This indicates that stdin should be closed
        // at this point
        do_send_stdin({});
    }

protected:
    /// Returns reference to executor for posting events
    auto & event_exec() { return event_exec_; }


    /// Returns reference to boost process child object
    boost::process::child & bp_child() { return bp_child_; }


    /// Waits for process exit and sends exit signal
    void send_exit_signal() {
        assert(all_buffers_cleared() && "buffers should be cleared before sending exit signal");

        // waiting for process exit (should be returned in short time becuase
        // process is already exited
        bp_child_.join();

        int exit_code = bp_child_.exit_code();

        // sending process exit event to main queue
        boost::asio::post(event_exec_, [this, exit_code]() {
            // executed in main thread
            exited_ = true;
            exited(exit_code);
        });
    }

    /// Starts writing stdin
    void start_write_stdin() {
        // executed in pool thread

        assert(!stdin_buffers_.empty() && "no buffers to write");

        CXXDBG_PROC_LOG_TRACE << "child " << this << " start write stdin"
                            << ", sz = " << stdin_buffers_.front().size();

        // checking for empty buffer to write. This indicates that stdin should be closed
        if (stdin_buffers_.front().empty()) {
            stdin_buffers_.pop_front();

            CXXDBG_PROC_LOG_TRACE << "child " << this << " close stdin";
            stdin_sink_.close();

            // sending exit signal if all buffers were cleared
            if (all_buffers_cleared()) {
                send_exit_signal();
            }

            return;
        }

        boost::asio::async_write(stdin_sink_,
                                    boost::asio::buffer(stdin_buffers_.front()),
                                    [this](auto && ecode, auto && size) {

            // executed in pool thread

            CXXDBG_PROC_LOG_TRACE << "child " << this << " write stdin complete: "
                                << ecode.value() << ", " << size;

            if (ecode.value() != 0 || size != stdin_buffers_.front().size()) {
                // error happened while writing data.
                // This indicates that process has exited.

                CXXDBG_PROC_LOG_TRACE << "child " << this << " write stdin error";

                // removing all stdin buffers to indicate that we are not sending data
                // anymore
                stdin_buffers_.clear();

                // sending exit signal if all buffers were cleared
                if (all_buffers_cleared()) {
                    send_exit_signal();
                }
            } else {
                // first buffer was sent, removing it from list of buffers
                stdin_buffers_.pop_front();

                // sending next stdin buffer if any
                if (!stdin_buffers_.empty()) {
                    CXXDBG_PROC_LOG_TRACE << "child " << this << " write next stdin buffer";
                    start_write_stdin();
                } else {
                    CXXDBG_PROC_LOG_TRACE << "child " << this << " end of stdin buffers";

                    // sending exit signal if all buffers were cleared
                    if (all_buffers_cleared()) {
                        send_exit_signal();
                    }
                }
            }
        });
    }


    /// Starts reading stdout
    void start_read_stdout() {
        // executed in pool thread or in main thread on first call

        CXXDBG_PROC_LOG_TRACE << "child " << this << " start read stdout";

        stdout_source_.async_read_some(boost::asio::buffer(stdout_buffer_),
                                        [this](auto && ecode, auto && size) {
            // executed in pool thread

            CXXDBG_PROC_LOG_TRACE << "child " << this << " read stdout complete: "
                                << ecode.value() << ", " << size;

            if (size != 0) {
                // sending notification in main queue
                std::string str(&stdout_buffer_[0], size);
                boost::asio::post(event_exec_, [this, str] {
                    // executed in main thread
                    stdout_received(str);
                });
            }

            if (ecode.value() != 0 || size == 0) {
                // Process exited, clearing stdout buffer to indicate it's not used anymore
                stdout_buffer_.clear();

                // sending exit signal if all buffers cleared
                if (all_buffers_cleared()) {
                    send_exit_signal();
                }
            } else {
                // reading stdout again
                start_read_stdout();
            }
        });
    }


    /// Returns true if all buffers were cleared and
    /// process should be considered as exited
    virtual bool all_buffers_cleared() const {
        return stdin_buffers_.empty() && stdout_buffer_.empty();
    }

private:
    /// Sends buffer to childs stdin
    void do_send_stdin(const std::string & data) {
        // executed in main thread
        boost::asio::post(io_ctx_, [this, data] {
            // executed in pool thread

            // detecting if we are already sending to stdin. In that case
            // list of buffers is not empty
            bool write_in_progress = !stdin_buffers_.empty();

            // adding data into list of buffers
            stdin_buffers_.push_back(data);

            // start sending data if not started yet
            if (!write_in_progress) {
                start_write_stdin();
            }
        });
    }

    boost::asio::io_context & io_ctx_;          ///< Reference to IO context
    boost::asio::executor event_exec_;          ///< Executor for posting events
    boost::process::child bp_child_;            ///< Boost process child object
    StdinSinkType & stdin_sink_;                ///< Reference to stdin sink
    StdoutSourceType & stdout_source_;          ///< Reference to stdout source

    std::list<std::string> stdin_buffers_;      ///< List of buffers to write to stdin
    std::vector<char> stdout_buffer_;           ///< Buffer for reading stdout

    bool exited_ = false;                       ///< Exited flag
};


}
