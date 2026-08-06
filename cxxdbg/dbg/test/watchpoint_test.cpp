// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file watchpoint_test.cpp
/// Contains unit tests for the watchpoint class.

#include "../watchpoint.hpp"
#include "../breakpoint_info.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::test {


class mock_watchpoint: public watchpoint {
public:
    mock_watchpoint(num_t n):
        watchpoint{n} {}

    ~mock_watchpoint() override {}

    std::string name() const override {
        return "mock_wp";
    }

    void before_start() override {
        watchpoint::before_start();
    }

    watchpoint_type type() const override { return type_var_wpoint; }
};


struct watchpoint_test_fixture {
    mock_watchpoint wp{10};
};



BOOST_FIXTURE_TEST_SUITE(watchpoint_test, watchpoint_test_fixture)


/// Tests updating current hit count and real current hit count in before_start
BOOST_AUTO_TEST_CASE(update_hit_count_before_start) {
    // increasing current hit count via update
    {
        watchpoint_info info{10};
        info.set_curr_hit_count(20);
        wp.update(&info);
    }

    BOOST_CHECK_EQUAL(wp.curr_hit_count(), 20);

    // increasing current hit count via update
    {
        watchpoint_info info{10};
        info.set_curr_hit_count(30);
        wp.update(&info);
    }

    BOOST_CHECK_EQUAL(wp.curr_hit_count(), 30);

    // resetting hit count via before_start
    wp.before_start();

    BOOST_CHECK_EQUAL(wp.curr_hit_count(), 0);


    // testing update with resetted real hit count
    // (it should be resetted in watchpoint before_start)
    {
        watchpoint_info info{10};
        info.set_curr_hit_count(5);
        wp.update(&info);
    }

    BOOST_CHECK_EQUAL(wp.curr_hit_count(), 5);
}


BOOST_AUTO_TEST_SUITE_END()


}
