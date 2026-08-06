// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file bp_launcher_test.cpp
/// Contains unit tests for the bp_launcher class.

#include "../bp_io_launcher.hpp"
#include "cxxdbg/async/simple_event_queue.hpp"
#include "cxxdbg/proc/launch_parameters.hpp"
#include "cxxdbg/util/boost_asio_io_pool.hpp"
#include <boost/test/unit_test.hpp>


#ifdef _WIN32
static auto work_dir = L"C:\\";
static auto ls_path = L"C:\\msys64\\usr\\bin\\ls.exe";
static auto cat_path = L"C:\\msys64\\usr\\bin\\cat.exe";
#elif defined(__APPLE__)
static auto work_dir = L"/";
static auto ls_path = L"/bin/ls";
static auto cat_path = L"/bin/cat";
#else
static auto work_dir = L"/";
static auto ls_path = L"/usr/bin/ls";
static auto cat_path = L"/usr/bin/cat";
#endif


namespace cxxdbg::proc::test {


struct bp_launcher_test_fixture {
    util::boost_asio_io_pool io_pool;
    async::simple_event_queue main_queue;
    bp_io_launcher launcher{io_pool.io_ctx(), main_queue.get_executor()};
};


BOOST_FIXTURE_TEST_SUITE(bp_launcher_test, bp_launcher_test_fixture)


/// Test construction and destruction
BOOST_AUTO_TEST_CASE(ctor_dtor) {
}


/// Tests simple process launch
BOOST_AUTO_TEST_CASE(launch) {
    // launching process
    launch_parameters pars;
    pars.cmd = ls_path;
    pars.work_dir = work_dir;
    auto child = launcher.launch_io(pars);

    // connecting to child signals
    int n_exited_called = 0;
    child->exited.connect([this, &n_exited_called](int ecode) {
        ++n_exited_called;

        // stopping processing events in main queue
        main_queue.post({});
    });

    int n_stdout_received_called = 0;
    child->stdout_received.connect([&n_stdout_received_called](auto && str) {
        ++n_stdout_received_called;
    });

    int n_stderr_received_called = 0;
    child->stderr_received.connect([&n_stderr_received_called](auto && str) {
        ++n_stderr_received_called;
    });

    // processing events in main queue
    main_queue.run();

    BOOST_CHECK_EQUAL(n_exited_called, 1);
    BOOST_CHECK(n_stdout_received_called > 0);
    BOOST_CHECK_EQUAL(n_stderr_received_called, 0);
}



/// Tests sending stdin to /usr/bin/cat
BOOST_AUTO_TEST_CASE(send_stdin) {
    // launching process
    launch_parameters pars;
    pars.cmd = cat_path;
    pars.work_dir = work_dir;
    auto child = launcher.launch_io(pars);

    // connecting to child signals
    int n_exited_called = 0;
    child->exited.connect([this, &n_exited_called](int ecode) {
        ++n_exited_called;

        // stopping processing events in main queue
        main_queue.post({});
    });

    std::string output;
    child->stdout_received.connect([&output](auto && str) {
        output += str;
    });

    child->stderr_received.connect([](auto &&) {
        BOOST_CHECK(false);
    });

    child->send_stdin("test output");
    child->send_stdin(" more output");
    child->close_stdin();

    // processing events in main queue
    main_queue.run();

    BOOST_CHECK_EQUAL(n_exited_called, 1);
    BOOST_CHECK_EQUAL(output, "test output more output");
}


BOOST_AUTO_TEST_SUITE_END()


}
