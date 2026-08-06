// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file bp_pty_launcher_test.cpp
/// Contains unit tests for the bp_pty_launcher class.

#include "../bp_pty_launcher.hpp"
#include "cxxdbg/async/simple_event_queue.hpp"
#include "cxxdbg/util/boost_asio_io_pool.hpp"
#include <boost/test/unit_test.hpp>

#ifdef _WIN32
#include <windows.h>
#endif


#ifdef _WIN32
static auto cat_path = L"C:\\msys64\\usr\\bin\\cat.exe";
#elif defined(__APPLE__)
static auto cat_path = L"/bin/cat";
#else
static auto cat_path = L"/usr/bin/cat";
#endif


namespace cxxdbg::proc::test {


struct bp_pty_launcher_test_fixture {
    util::boost_asio_io_pool io_pool;
    async::simple_event_queue main_queue;
    bp_pty_launcher launcher{io_pool.io_ctx(), main_queue.get_executor()};
};


BOOST_FIXTURE_TEST_SUITE(bp_pty_launcher_test, bp_pty_launcher_test_fixture)


/// Tests launching /usr/bin/cat in PTY
BOOST_AUTO_TEST_CASE(launch_pty) {
    // launching process
    launch_parameters pars;
    pars.exe = cat_path;
    pars.work_dir = L"/";
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
#ifdef _WIN32
    ::Sleep(100);
#else
    ::sleep(1);
#endif
    child->close_stdin();

    // processing events in main queue
    main_queue.run();

    BOOST_CHECK_EQUAL(n_exited_called, 1);
    BOOST_CHECK_EQUAL(output, "test output more output");
}


BOOST_AUTO_TEST_SUITE_END()


}
