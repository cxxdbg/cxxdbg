// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file boost_asio_io_pool.hpp
/// Contains definition of the boost_asio_io_pool class.

#pragma once

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <thread>


namespace cxxdbg::util {


/// Incapsulates common code that initializes ASIO IO context and single thread
/// for executing async operations in this context
class boost_asio_io_pool {
public:
    /// Initializes ASIO IO context and work thread
    boost_asio_io_pool();

    /// Stops work thread and destroys IO context
    ~boost_asio_io_pool();

    /// Returns reference to initialized IO context
    auto & io_ctx() { return io_ctx_; }

private:
    using work_guard_type =
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

    boost::asio::io_context io_ctx_;            ///< ASIO IO context for ASIO related async operations
    std::thread io_thread_;                     ///< Thread for processing IO operations

    /// Work guard for making io_context run not return until stop is called
    std::unique_ptr<work_guard_type> io_ctx_work_guard_;
};


}
