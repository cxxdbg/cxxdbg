// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file async_pty.hpp
/// Contains definition of the async_pty class.

#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include "cxxdbg/boost_process/process.hpp"


namespace cxxdbg::proc {


/// Async PTY implementation for boost asio
class async_pty {
    using pty_master_type = ::boost::asio::posix::stream_descriptor;
    using io_context = ::boost::asio::io_context;

public:
    /// Type of executor associated with stream
    using executor_type = pty_master_type::executor_type;

    /// Constructs async PTY with specified IO context for input and output
    explicit async_pty(io_context & ctx);

    // noncopyable
    async_pty(const async_pty &) = delete;
    async_pty & operator=(const async_pty &) = delete;

    /// Move constructor
    async_pty(async_pty && pty);

    /// Destroys async PTY
    ~async_pty();

    /// Move assign operator
    async_pty & operator=(async_pty && pty);

    /// Opens slave terminal for master terminal. Returns opened fd
    int open_slave();

    /// Returns executor associated with stream
    executor_type get_executor() {
        return pty_master_.get_executor();
    }

    /// Reads bytes from terminal
    template<typename MutableBufferSequence,
             typename ReadHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
          ReadHandler, void(boost::system::error_code, std::size_t))
      async_read_some(
        const MutableBufferSequence & buffers,
              ReadHandler &&handler)
    {
        pty_master_.async_read_some(buffers, std::forward<ReadHandler>(handler));
    }


    /// Writes bytes to terminal
    template<typename ConstBufferSequence,
             typename WriteHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
              WriteHandler, void(boost::system::error_code, std::size_t))
      async_write_some(
        const ConstBufferSequence & buffers,
        WriteHandler&& handler)
    {
        pty_master_.async_write_some(buffers, std::forward<WriteHandler>(handler));
    }

    void close() {
        if (pty_master_.is_open()) {
            pty_master_.close();
        }
    }

private:
    pty_master_type pty_master_;
};


}

