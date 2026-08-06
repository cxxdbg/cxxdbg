// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file code_breakpoint_test.cpp
/// Contains unit tests for the code_breakpoint class.

#include "../breakpoint_location.hpp"
#include "../code_breakpoint.hpp"
#include "../code_position.hpp"
#include "../source_model.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::test {


#ifdef _WIN32
static const std::string p_prefix = "C:/";
#else
static const std::string p_prefix = "/";
#endif


class mock_code_breakpoint: public code_breakpoint {
public:
    mock_code_breakpoint(source_model & src_mdl, num_t n):
        code_breakpoint{src_mdl, n} {}

    ~mock_code_breakpoint() override {}

    std::string name() const override {
        return "mock_code_bp_name";
    }

    void before_start() override {
        code_breakpoint::before_start();
    }

    void install_into_impl(breakpoint_list_impl &, const install_handler & h) override {
        h(code_breakpoint_info{-1});
    }
};


struct code_breakpoint_test_fixture {
    source_model src_model;
    mock_code_breakpoint bp{src_model, 10};
};


BOOST_FIXTURE_TEST_SUITE(code_breakpoint_test, code_breakpoint_test_fixture)


/// Tests adding locations
BOOST_AUTO_TEST_CASE(test_add_locations) {
    BOOST_CHECK(std::ranges::size(bp.locations()) == 0);
    BOOST_CHECK(std::ranges::empty(bp.locations()));

    code_position cpos{10, 20, "func", source_position{}};
    std::shared_ptr<breakpoint_location> loc{new breakpoint_location{&bp, 10, cpos}};
    bp.add_location(loc);

    code_position cpos2{30, 50, "func2", source_position{}};
    std::shared_ptr<breakpoint_location> loc2{new breakpoint_location{&bp, 12, cpos2}};
    bp.add_location(loc2);

    BOOST_REQUIRE(bp.locations_size() == 2);

    auto locs = bp.locations();
    auto it = std::ranges::begin(locs);
    BOOST_REQUIRE(it != std::ranges::end(locs));
    BOOST_CHECK((*it)->id() == 10);


    ++it;
    BOOST_REQUIRE(it != std::ranges::end(locs));
    BOOST_CHECK((*it)->id() == 12);

    ++it;
    BOOST_CHECK(it == std::ranges::end(locs));
}


/// Tests removing location by id
BOOST_AUTO_TEST_CASE(test_remove_locations) {
    BOOST_CHECK(std::ranges::size(bp.locations()) == 0);

    code_position cpos{10, 20, "func", source_position{}};
    std::shared_ptr<breakpoint_location> loc{new breakpoint_location{&bp, 10, cpos}};
    bp.add_location(loc);

    code_position cpos2{30, 50, "func2", source_position{}};
    std::shared_ptr<breakpoint_location> loc2{new breakpoint_location{&bp, 12, cpos2}};
    bp.add_location(loc2);

    BOOST_REQUIRE(bp.locations_size() == 2);

    bp.remove_location(10);

    BOOST_REQUIRE(bp.locations_size() == 1);

    auto locs = bp.locations();
    auto it = std::ranges::begin(locs);
    BOOST_REQUIRE(it != std::ranges::end(locs));
    BOOST_CHECK((*it)->id() == 12);

    ++it;
    BOOST_CHECK(it == std::ranges::end(locs));
}


/// Tests seaching location by id
BOOST_AUTO_TEST_CASE(test_search_locations) {
    BOOST_CHECK(std::ranges::empty(bp.locations()));

    code_position cpos{10, 20, "func", source_position{}};
    std::shared_ptr<breakpoint_location> loc{new breakpoint_location{&bp, 10, cpos}};
    bp.add_location(loc);

    code_position cpos2{30, 50, "func2", source_position{}};
    std::shared_ptr<breakpoint_location> loc2{new breakpoint_location{&bp, 12, cpos2}};
    bp.add_location(loc2);

    BOOST_REQUIRE(bp.locations_size() == 2);

    auto l = bp.find_location(12);
    BOOST_CHECK(l->id() == 12);
}


/// Tests adding locations via update
BOOST_AUTO_TEST_CASE(test_update_add_loc) {
    code_breakpoint_info bpinf{10};
    bpinf.add_location({12, {10, 20, "func", {p_prefix + "test.cpp", 77}}});
    bpinf.add_location({13, {40, 50, "f2", {p_prefix + "foo.cpp", 88}}});
    bool res = bp.update(&bpinf);

    BOOST_CHECK(res);
    BOOST_CHECK(bp.locations_size() == 2);

    auto locs = bp.locations();
    auto it = std::ranges::begin(locs);
    BOOST_REQUIRE(it != std::ranges::end(locs));
    BOOST_CHECK((*it)->id() == 12);
    BOOST_CHECK((*it)->pos().load_addr() == 10);
    BOOST_CHECK((*it)->pos().file_addr() == 20);
    BOOST_CHECK((*it)->pos().func_name() == "func");
    BOOST_CHECK((*it)->pos().src_pos().file()->path() == p_prefix + "test.cpp");

    BOOST_CHECK((*it)->pos().src_pos().line() == 77);

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(locs));
    BOOST_CHECK((*it)->id() == 13);
    BOOST_CHECK((*it)->pos().load_addr() == 40);
    BOOST_CHECK((*it)->pos().file_addr() == 50);
    BOOST_CHECK((*it)->pos().func_name() == "f2");
    BOOST_CHECK((*it)->pos().src_pos().file()->path() == p_prefix + "foo.cpp");
    BOOST_CHECK((*it)->pos().src_pos().line() == 88);

    ++it;
    BOOST_CHECK(it == std::ranges::end(locs));

    bpinf.add_location({7, {1, 2, "f3", {p_prefix + "bar.cpp", 678}}});
    bp.update(&bpinf);


    it = std::ranges::begin(locs);
    BOOST_REQUIRE(it != std::ranges::end(locs));
    BOOST_CHECK((*it)->id() == 12);
    BOOST_CHECK((*it)->pos().load_addr() == 10);
    BOOST_CHECK((*it)->pos().file_addr() == 20);
    BOOST_CHECK((*it)->pos().func_name() == "func");
    BOOST_CHECK((*it)->pos().src_pos().file()->path() == p_prefix + "test.cpp");
    BOOST_CHECK((*it)->pos().src_pos().line() == 77);

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(locs));
    BOOST_CHECK((*it)->id() == 13);
    BOOST_CHECK((*it)->pos().load_addr() == 40);
    BOOST_CHECK((*it)->pos().file_addr() == 50);
    BOOST_CHECK((*it)->pos().func_name() == "f2");
    BOOST_CHECK((*it)->pos().src_pos().file()->path() == p_prefix + "foo.cpp");
    BOOST_CHECK((*it)->pos().src_pos().line() == 88);

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(locs));
    BOOST_CHECK((*it)->id() == 7);
    BOOST_CHECK((*it)->pos().load_addr() == 1);
    BOOST_CHECK((*it)->pos().file_addr() == 2);
    BOOST_CHECK((*it)->pos().func_name() == "f3");
    BOOST_CHECK((*it)->pos().src_pos().file()->path() == p_prefix + "bar.cpp");
    BOOST_CHECK((*it)->pos().src_pos().line() == 678);

    ++it;
    BOOST_CHECK(it == std::ranges::end(locs));
}


/// Tests removing locations via update
BOOST_AUTO_TEST_CASE(test_update_remove_loc) {
    code_breakpoint_info bpinf{10};
    bpinf.add_location({12, {10, 20, "func", {p_prefix + "test.cpp", 77}}});
    bpinf.add_location({13, {40, 50, "f2", {p_prefix + "foo.cpp", 88}}});
    bp.update(&bpinf);

    code_breakpoint_info bpinf2{10};
    bpinf2.add_location({12, {11, 21, "func1", {p_prefix + "test1.cpp", 771}}});
    bool res = bp.update(&bpinf2);

    BOOST_CHECK(res);
    BOOST_CHECK(bp.locations_size() == 1);

    auto locs = bp.locations();
    auto it = std::ranges::begin(locs);
    BOOST_REQUIRE(it != std::ranges::end(locs));
    BOOST_CHECK((*it)->id() == 12);
    BOOST_CHECK((*it)->pos().load_addr() == 11);
    BOOST_CHECK((*it)->pos().file_addr() == 21);
    BOOST_CHECK((*it)->pos().func_name() == "func1");
    BOOST_CHECK((*it)->pos().src_pos().file()->path() == p_prefix + "test1.cpp");
    BOOST_CHECK((*it)->pos().src_pos().line() == 771);

    ++it;
    BOOST_CHECK(it == std::ranges::end(locs));
}


/// Tests resetting current hit count at start
BOOST_AUTO_TEST_CASE(test_reset_curr_real_hit_count) {
    BOOST_CHECK(bp.curr_hit_count() == 0);

    // updating current hit count to value 30
    {
        code_breakpoint_info bpinf{10};
        bpinf.set_curr_hit_count(30);
        bp.update(&bpinf);
    }

    BOOST_CHECK(bp.curr_hit_count() == 30);

    // testing resetting current hit count in before_start
    bp.before_start();

    BOOST_CHECK(bp.curr_hit_count() == 0);
}


/// Tests updating hit count via update
BOOST_AUTO_TEST_CASE(test_update_hit_count) {
    BOOST_CHECK(bp.curr_hit_count() == 0);

    code_breakpoint_info bpinf{10};
    bpinf.set_curr_hit_count(30);
    bool res = bp.update(&bpinf);

    BOOST_CHECK(res);
    BOOST_CHECK(bp.curr_hit_count() == 30);

    // resetting current hit count
    bp.before_start();

    BOOST_CHECK(bp.curr_hit_count() == 0);

    bpinf.set_curr_hit_count(50);
    res = bp.update(&bpinf);

    BOOST_CHECK(res);
    BOOST_CHECK(bp.curr_hit_count() == 20);
}


BOOST_AUTO_TEST_SUITE_END()


}
