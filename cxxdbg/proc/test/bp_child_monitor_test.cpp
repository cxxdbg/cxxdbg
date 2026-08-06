// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file bp_child_monitor_test.cpp
/// Contains unit tests for the bp_child_monitor class.

#include "../bp_child_monitor.hpp"
#include "cxxdbg/async/simple_event_queue.hpp"
#include <boost/test/unit_test.hpp>
#include <thread>


namespace bp = boost::process;


#ifdef _WIN32
static auto ls_path = L"C:\\msys64\\usr\\bin\\ls.exe";
static auto cat_path = L"C:\\msys64\\usr\\bin\\cat.exe";
#elif defined(__APPLE__)
static auto ls_path = L"/bin/ls";
static auto cat_path = L"/bin/cat";
#else
static auto ls_path = L"/usr/bin/ls";
static auto cat_path = L"/usr/bin/cat";
#endif


namespace cxxdbg::proc::test {


BOOST_AUTO_TEST_SUITE(bp_child_monitor_test)


/// Tests launching and monitoring /bin/ls
BOOST_AUTO_TEST_CASE(test_ls_monitor) {
    async::simple_event_queue main_queue;

    // launching child process
    bp::child child{ls_path};

    // creating monitor
    bp_child_monitor child_mon{main_queue.get_executor(), std::move(child)};

    // listening for exit signal
    int n_exited_called = 0;
    child_mon.exited.connect([this, &main_queue, &n_exited_called](int ecode) {
        BOOST_CHECK_EQUAL(ecode, 0);
        ++n_exited_called;
        main_queue.post({});
    });

    // processing events in main queue
    main_queue.run();

    // checking that exit signal was called
    BOOST_CHECK_EQUAL(n_exited_called, 1);
}


/// Tests launching and monitoring /usr/bin/cat with manual termination
BOOST_AUTO_TEST_CASE(test_cat_monitor) {
    async::simple_event_queue main_queue;

    // launching child process
    bp::child child{cat_path};

    // creating monitor
    bp_child_monitor child_mon{main_queue.get_executor(), std::move(child)};

    // listening for exit signal
    int n_exited_called = 0;
    child_mon.exited.connect([this, &main_queue, &n_exited_called](int ecode) {
        BOOST_CHECK(ecode != 0);
        ++n_exited_called;
        main_queue.post({});
    });

    // waiting for 1 second
    std::this_thread::sleep_for(std::chrono::seconds{1});

    // terminating process
    child_mon.terminate();

    // processing events in main queue
    main_queue.run();

    // checking that exit signal was called
    BOOST_CHECK_EQUAL(n_exited_called, 1);
}


BOOST_AUTO_TEST_SUITE_END()


}
