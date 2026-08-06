// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file watchpoint_group_test.cpp
/// Contains unit tests for the watchpoint_group class.

#include "mock_exec_processor.hpp"
#include "mock_watch_processor.hpp"
#include "../watch_group.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::cli::test {


struct watch_group_test_fixture {
    mock_exec_processor exec_proc;
    mock_watch_processor watch_proc;
    watch_group grp{exec_proc, watch_proc};
};


BOOST_FIXTURE_TEST_SUITE(watchpoint_group_test, watch_group_test_fixture)


/// Tests adding variable watchpoint
BOOST_AUTO_TEST_CASE(add_var) {
    MOCK_ADD_CALL(watch_proc, add_var_watchpoint, [](auto && name,
                                                     auto && props,
                                                     auto && h) {
        BOOST_CHECK(name == "my_var");
        BOOST_CHECK(!props.is_read);
        BOOST_CHECK(props.is_write);
        BOOST_CHECK_EQUAL(props.size, 0);
        BOOST_CHECK(!props.enable_hit_count);
        BOOST_CHECK_EQUAL(props.cond, "");
        h(async::ok_result(12));
    });

    bool hcalled = false;
    grp.exec({"set", "variable", "my_var"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "added watchpoint 12");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(watch_proc.verify());
}


/// Tests adding expression watchpoint
BOOST_AUTO_TEST_CASE(add_expr) {
    MOCK_ADD_CALL(watch_proc, add_expr_watchpoint, [](auto && expr,
                                                      auto && props,
                                                      auto && h) {
        BOOST_CHECK(expr == "my_expr");
        BOOST_CHECK(!props.is_read);
        BOOST_CHECK(props.is_write);
        BOOST_CHECK_EQUAL(props.size, 0);
        BOOST_CHECK(!props.enable_hit_count);
        BOOST_CHECK_EQUAL(props.cond, "");
        h(async::ok_result(12));
    });

    bool hcalled = false;
    grp.exec({"set", "expression", "my_expr"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "added watchpoint 12");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(watch_proc.verify());
}


/// Tests adding expression watchpoint with additional flags
BOOST_AUTO_TEST_CASE(add_expr_opts) {
    MOCK_ADD_CALL(watch_proc, add_expr_watchpoint, [](auto && expr,
                                                      auto && props,
                                                      auto && h) {
        BOOST_CHECK(expr == "my_expr");
        BOOST_CHECK(props.is_read);
        BOOST_CHECK(props.is_write);
        BOOST_CHECK_EQUAL(props.size, 8);
        BOOST_CHECK(!props.enable_hit_count);
        BOOST_CHECK_EQUAL(props.cond, "");
        h(async::ok_result(12));
    });

    bool hcalled = false;
    grp.exec({"set", "expression", "my_expr", "-w", "read_write", "-s", "8"},
             [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "added watchpoint 12");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(watch_proc.verify());
}


/// Tests passing invalid size to add watchpoint command
BOOST_AUTO_TEST_CASE(add_expr_invalid_size) {
    bool hcalled = false;
    grp.exec({"set", "expression", "my_expr", "-w", "read_write", "-s", "20"},
             [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "invalid watch size: 20");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(watch_proc.verify());
}



BOOST_AUTO_TEST_SUITE_END()


}
