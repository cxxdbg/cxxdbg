// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "../exec_stub_client.hpp"
#include "../exec_stub_server.hpp"
#include <thread>
#include "cxxdbg/boost_process/process.hpp"
#include <boost/test/unit_test.hpp>

/// Contains unit tests for exec_stub_server / exec_stub_client classes

namespace cxxdbg::dbg::exec_stub {


BOOST_AUTO_TEST_SUITE(exec_stub_server_client_test)


/// Tests server initialization
BOOST_AUTO_TEST_CASE(server_init) {
    exec_stub_server srv;

    BOOST_CHECK(!srv.shared_mem_name().empty());
}


/// Tests receiving PID timeout
BOOST_AUTO_TEST_CASE(wait_timeout) {
    exec_stub_server srv;

    auto [pid, tid] = srv.wait_pid(std::chrono::seconds(1));
    BOOST_CHECK(pid == 0);
    BOOST_CHECK(tid == 0);
}


/// Tests sending / receiving custom PID
BOOST_AUTO_TEST_CASE(send_receive) {
    exec_stub_server srv;
    std::string shm_name = srv.shared_mem_name();

    // starting thread which will send PID
    std::thread thrd([shm_name] {
        // waiting 2 seconds before sending PID
        std::this_thread::sleep_for(std::chrono::seconds(2));

        exec_stub_client client(shm_name);
        client.send_pid(300, 400);
    });

    // waiting for PID
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
    auto [pid, tid] = srv.wait_pid(std::chrono::seconds(10));

    // checking that wait time is less than 5 seconds
    std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
    std::chrono::seconds secs =
            std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
    BOOST_CHECK(secs.count() < 5);

    BOOST_CHECK(pid == 300);
    BOOST_CHECK(tid == 400);

    thrd.join();
}



/// Tests sending / receiving current PID
BOOST_AUTO_TEST_CASE(send_receive_curr) {
    exec_stub_server srv;
    std::string shm_name = srv.shared_mem_name();

    // starting thread which will send PID
    std::thread thrd([shm_name] {
        // waiting 2 seconds before sending PID
        std::this_thread::sleep_for(std::chrono::seconds(2));

        exec_stub_client client(shm_name);
        client.send_pid();
    });

    // waiting for PID
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
    auto [pid, tid] = srv.wait_pid(std::chrono::seconds(10));

    // checking that wait time is less than 5 seconds
    std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
    std::chrono::seconds secs =
            std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
    BOOST_CHECK(secs.count() < 5);

    BOOST_CHECK(pid == boost::this_process::get_id());
    BOOST_CHECK(tid == 0);

    thrd.join();
}


BOOST_AUTO_TEST_SUITE_END()


}
