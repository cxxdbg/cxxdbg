// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file breakpoint_test.cpp
/// Contains unit tests for the breakpoint class.

#include "cxxdbg/dbg/breakpoint_info.hpp"
#include "cxxdbg/dbg/breakpoint_location.hpp"
#include "cxxdbg/dbg/code_breakpoint.hpp"
#include "cxxdbg/dbg/code_position.hpp"
#include "cxxdbg/dbg/source_file.hpp"
#include "cxxdbg/dbg/source_model.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::test {

namespace {

class mock_breakpoint: public breakpoint {
public:
    mock_breakpoint(num_t n):
        breakpoint{n} {}

    ~mock_breakpoint() override {}

    std::string name() const override {
        return "mock_bp_name";
    }

    const breakpoint_site * child_at(std::size_t position) const override {
        return nullptr;
    }

    std::size_t children_size() const override {
        return 0;
    }

    const breakpoint_location * get_single_location() const override {
        return nullptr;
    }

    source_position get_source_position() const override {
        return {};
    }
};

}


class mock_breakpoint_info: public breakpoint_info {
public:
    mock_breakpoint_info(num_t n): breakpoint_info{n} {}
    breakpoint_type type() const override { return breakpoint_type::code; }
};


struct breakpoint_test_fixture {
    source_model src_model;
    mock_breakpoint bp{10};
};


BOOST_FIXTURE_TEST_SUITE(breakpoint_test, breakpoint_test_fixture)


/// Initialization test
BOOST_AUTO_TEST_CASE(test_init) {
    BOOST_CHECK(bp.num() == 10);
    BOOST_CHECK(bp.enabled());
    BOOST_CHECK(bp.condition() == "");
    BOOST_CHECK(bp.curr_hit_count() == 0);
}


/// set_enabled / enabled test
BOOST_AUTO_TEST_CASE(enabled_test) {
    BOOST_CHECK(bp.enabled());
    bp.set_enabled(false);
    BOOST_CHECK(!bp.enabled());
    bp.set_enabled(true);
    BOOST_CHECK(bp.enabled());
}


/// set_condition / condition test
BOOST_AUTO_TEST_CASE(condition_test) {
    BOOST_CHECK(bp.condition() == "");
    bp.set_condition("aaaa");
    BOOST_CHECK(bp.condition() == "aaaa");
}


/// Tests hit count disabled after initialization
BOOST_AUTO_TEST_CASE(hit_count_init_disabled) {
    BOOST_CHECK(!bp.hit_count_enabled());
}


/// Tests set_hit_count_enabled with true
BOOST_AUTO_TEST_CASE(test_set_hit_count_enabled) {
    bp.set_hit_count_enabled(true);
    BOOST_CHECK(bp.hit_count_enabled());
}


/// Tests set_hit_count_enabled with false
BOOST_AUTO_TEST_CASE(test_set_hit_count_enabled_false) {
    bp.set_hit_count_enabled(false);
    BOOST_CHECK(!bp.hit_count_enabled());
}


/// Tests hit count property initialization
BOOST_AUTO_TEST_CASE(test_hit_count_init) {
    bp.set_hit_count_enabled(false);
    BOOST_CHECK(bp.hit_count() == 1);
}


/// Tests setting hit count property
BOOST_AUTO_TEST_CASE(test_set_hit_count) {
    bp.set_hit_count(50);
    BOOST_CHECK(bp.hit_count() == 50);
}


/// Tests updating with same info
BOOST_AUTO_TEST_CASE(test_update_same) {
    code_breakpoint_info inf{10};
    bool res = bp.update(&inf);
    BOOST_CHECK(!res);
    BOOST_CHECK(bp.curr_hit_count() == 0);
}


/// Tests calculating ignore count when hit count is disabled
BOOST_AUTO_TEST_CASE(test_ignore_count_disabled) {
    code_breakpoint_info inf{10};
    BOOST_CHECK_EQUAL(bp.ignore_count(), 0);

    {
        mock_breakpoint_info bpinf{10};
        bpinf.set_curr_hit_count(20);
        bp.update(&bpinf);
    }

    BOOST_CHECK_EQUAL(bp.ignore_count(), 0);
}


/// Tests calculating ignore count when hit count is enabled
BOOST_AUTO_TEST_CASE(test_ignore_count_enabled) {
    code_breakpoint_info inf{10};
    BOOST_CHECK_EQUAL(bp.ignore_count(), 0);

    bp.set_hit_count(20);
    bp.set_hit_count_enabled(true);
    BOOST_CHECK_EQUAL(bp.ignore_count(), 19);

    {
        mock_breakpoint_info bpinf{10};
        bpinf.set_curr_hit_count(10);
        bp.update(&bpinf);
    }

    BOOST_CHECK_EQUAL(bp.ignore_count(), 9);
}


/// Tests calculating ignore count after resetting current hit count
BOOST_AUTO_TEST_CASE(test_ignore_count_after_reset) {
    code_breakpoint_info inf{10};

    {
        mock_breakpoint_info bpinf{10};
        bpinf.set_curr_hit_count(10);
        bp.update(&bpinf);
    }

    bp.reset_curr_hit_count();
    bp.set_hit_count(20);
    bp.set_hit_count_enabled(true);
    BOOST_CHECK_EQUAL(bp.ignore_count(), 19);
}


/// Tests calculating ignore count after when current hit count > hit count
BOOST_AUTO_TEST_CASE(test_ignore_count_chc_gt_hc) {
    code_breakpoint_info inf{10};

    bp.set_hit_count(5);
    bp.set_hit_count_enabled(true);

    {
        mock_breakpoint_info bpinf{10};
        bpinf.set_curr_hit_count(10);
        bp.update(&bpinf);
    }

    BOOST_CHECK_EQUAL(bp.ignore_count(), 0);
}


BOOST_AUTO_TEST_SUITE_END()


}
