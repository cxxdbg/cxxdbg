// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file win32_monitor_test.cpp
/// Contains unit tests for the win32_monitor class.

#include "../win32_monitor.hpp"
#include "cxxdbg/async/simple_event_queue.hpp"
#include "cxxdbg/boost_process/process.hpp"
#include <boost/test/unit_test.hpp>

namespace bp = boost::process;


static auto ls_path = L"C:\\msys64\\usr\\bin\\ls.exe";
static auto cat_path = L"C:\\msys64\\usr\\bin\\cat.exe";


namespace cxxdbg::proc::test {

BOOST_AUTO_TEST_SUITE(win32_monitor_test)


/// Tests monitoring process exit
BOOST_AUTO_TEST_CASE(monitor_exit) {
    async::simple_event_queue main_queue;
    
    // launching /usr/bin/cat
    bp::child child{cat_path};

    // creating posix monitor
    win32_monitor child_mon{main_queue.get_executor(), child.id()};

    // waiting 1 second
    std::this_thread::sleep_for(std::chrono::seconds{1});

    // checking that event queue is empty (monitor did not send any signals)
    BOOST_CHECK(main_queue.empty());

    // listening for exit signal
    int n_exited_called = 0;
    child_mon.exited.connect([&n_exited_called, &main_queue](int ecode) {
        ++n_exited_called;
        main_queue.post({});
    });

    // killing child
    child.terminate();

    // // we need call waitpid here because of bug in boost process that
    // // prevents use of wait method for removing child zombie
    // ::waitpid(child.id(), nullptr, 0);

    // monitoring events in main queue
    main_queue.run();

    BOOST_CHECK_EQUAL(n_exited_called, 1);
}


/// Tests terminating process
BOOST_AUTO_TEST_CASE(terminate) {
    async::simple_event_queue main_queue;
    
    // launching /usr/bin/cat
    bp::child child{ls_path};

    // creating posix monitor
    win32_monitor child_mon{main_queue.get_executor(), child.id()};

    // listening for exit signal
    int n_exited_called = 0;
    child_mon.exited.connect([&n_exited_called, &main_queue](int ecode) {
        ++n_exited_called;
        main_queue.post({});
    });

    // terminating process
    child_mon.terminate();

    // // we need call waitpid here because of bug in boost process that
    // // prevents use of wait method for removing child zombie
    // ::waitpid(child.id(), nullptr, 0);

    // monitoring events in main queue
    main_queue.run();

    BOOST_CHECK_EQUAL(n_exited_called, 1);
}


BOOST_AUTO_TEST_SUITE_END()

}