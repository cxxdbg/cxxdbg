// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file boost_asio_io_pool.cpp
/// Contains implementation of the boost_asio_io_pool class.

#include "boost_asio_io_pool.hpp"


namespace cxxdbg::util {


boost_asio_io_pool::boost_asio_io_pool() {
    io_ctx_work_guard_ = std::make_unique<work_guard_type>(io_ctx_.get_executor());

    // starting thread for processing IO
    io_thread_ = std::thread([this] {
        io_ctx_.run();
    });
}


boost_asio_io_pool::~boost_asio_io_pool() {
    // stopping ASIO IO context work thread
    io_ctx_work_guard_.reset();
    io_thread_.join();
}


}
