// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file event_queue_test.cpp
/// Contains unit tests for the event_queue class and its executor.

#include "cxxdbg/async/event_queue.hpp"
#include "cxxdbg/async/simple_event_queue.hpp"
#include <boost/asio/executor.hpp>
#include <boost/asio/post.hpp>
#include <boost/test/unit_test.hpp>
#include <memory>


namespace cxxdbg::async::test {


BOOST_AUTO_TEST_SUITE(result_test)


/// Tests constructing ASIO executor
BOOST_AUTO_TEST_CASE(asio_exec) {
    simple_event_queue eq;
    boost::asio::executor exec{eq.get_executor()};
    boost::asio::post(exec, [] {});
}


BOOST_AUTO_TEST_SUITE_END()

}
