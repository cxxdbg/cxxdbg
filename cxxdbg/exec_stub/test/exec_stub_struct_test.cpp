// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "../exec_stub_struct.hpp"
#include <chrono>
#include <thread>
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::exec_stub {


BOOST_AUTO_TEST_SUITE(exec_stub_struct_test)


BOOST_AUTO_TEST_CASE(init) {
    exec_stub_struct str;

    BOOST_CHECK(str.pid() == 0);
}


BOOST_AUTO_TEST_CASE(set_pid) {
    exec_stub_struct str;
    str.set_pid(100, 200);

    BOOST_CHECK(str.pid() == 100);
    BOOST_CHECK(str.tid() == 200);
}


/// Tests wait timeout
BOOST_AUTO_TEST_CASE(wait_timeout) {
    exec_stub_struct str;

    auto [pid, tid] = str.wait_for_pid(boost::posix_time::microseconds(2));
    BOOST_CHECK(pid == 0);
    BOOST_CHECK(tid == 0);
}


/// Tests wait after set_pid is called
BOOST_AUTO_TEST_CASE(wait_after_set_pid) {
    exec_stub_struct str;
    str.set_pid(100, 200);

    auto [pid, tid] = str.wait_for_pid(boost::posix_time::microseconds(0));
    BOOST_CHECK(pid == 100);
    BOOST_CHECK(tid == 200);
}


/// Tests wait / set_pid for 2 threads
BOOST_AUTO_TEST_CASE(wait_set_pid_threads) {
    exec_stub_struct str;

    std::thread thrd([&str] {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        str.set_pid(200, 300);
    });

    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
    auto [pid, tid] = str.wait_for_pid(boost::posix_time::seconds(10));

    // checking that wait time is less than 5 seconds
    std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
    std::chrono::seconds secs =
            std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
    BOOST_CHECK(secs.count() < 5);

    BOOST_CHECK(pid == 200);
    BOOST_CHECK(tid == 300);

    thrd.join();
}


BOOST_AUTO_TEST_SUITE_END()


}
