// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file concat_test.cpp
/// Contain unit tests for concat view for ranges.

#include <boost/test/unit_test.hpp>
#include <ranges.hpp>


#if __cplusplus > 201703L
static_assert(std::ranges::input_range<xranges::concat_view<std::ranges::views::all_t<std::vector<int>&>>>);
static_assert(std::ranges::view<xranges::concat_view<std::ranges::views::all_t<std::vector<int>&>>>);
#endif


namespace xranges::test {


struct concat_test_fixture {
    std::vector<int> v;
    std::vector<int> v2;
    std::list<int> l;
    decltype(v | xranges::views::concat(v2, l)) res = v | xranges::views::concat(v2, l);

    concat_test_fixture() {
        v.push_back(10);
        v.push_back(20);
        v2.push_back(30);
        l.push_back(40);
        l.push_back(50);
    }
};


BOOST_FIXTURE_TEST_SUITE(concat_test, concat_test_fixture)


/// Tests size of concat view
BOOST_AUTO_TEST_CASE(size) {
    BOOST_CHECK_EQUAL(std::ranges::size(res), 5);
}


/// Tests iteration over concat view
BOOST_AUTO_TEST_CASE(test_iter) {
    auto it = std::ranges::begin(res);
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK_EQUAL(*it, 10);

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK_EQUAL(*it, 20);

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK_EQUAL(*it, 30);

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK_EQUAL(*it, 40);

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK_EQUAL(*it, 50);

    ++it;
    BOOST_CHECK(it == std::ranges::end(res));
}


/// Tests chain of concat views
BOOST_AUTO_TEST_CASE(concat_chain) {
    auto tmp = views::concat(v, v2);
    auto res = views::concat(tmp, l);

    auto it = std::ranges::begin(res);
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK_EQUAL(*it, 10);

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK_EQUAL(*it, 20);

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK_EQUAL(*it, 30);

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK_EQUAL(*it, 40);

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK_EQUAL(*it, 50);

    ++it;
    BOOST_CHECK(it == std::ranges::end(res));
}


/// Tests iteration over const view
BOOST_AUTO_TEST_CASE(iter_const) {
    const auto & cres = res;

    auto it = std::ranges::begin(cres);
    BOOST_REQUIRE(it != std::ranges::end(cres));
    BOOST_CHECK_EQUAL(*it, 10);

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(cres));
    BOOST_CHECK_EQUAL(*it, 20);

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(cres));
    BOOST_CHECK_EQUAL(*it, 30);

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(cres));
    BOOST_CHECK_EQUAL(*it, 40);

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(cres));
    BOOST_CHECK_EQUAL(*it, 50);

    ++it;
    BOOST_CHECK(it == std::ranges::end(cres));
}


/// Tests concat view with transform view
BOOST_AUTO_TEST_CASE(concat_transform) {
    auto fn = [](int x) { return x * x; };
    auto tv = v | std::ranges::views::transform(fn);
    //auto tv2 = v2 | std::ranges::views::transform(fn);
    auto tl = l | std::ranges::views::transform(fn);

    auto res = views::concat(tv, tl);

    auto it = std::ranges::begin(res);
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK_EQUAL(*it, 100);

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK_EQUAL(*it, 400);

    // ++it;
    // BOOST_REQUIRE(it != std::ranges::end(res));
    // BOOST_CHECK_EQUAL(*it, 30);

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK_EQUAL(*it, 1600);

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK_EQUAL(*it, 2500);

    ++it;
    BOOST_CHECK(it == std::ranges::end(res));
}


/// Tests constructing string from concat view
BOOST_AUTO_TEST_CASE(test_string_construct) {
    std::vector<char> v1 = {'a', 'b'};
    std::vector<char> v2 = {'c', 'd'};

    auto view = views::concat(v1, v2);
    std::string res{std::ranges::begin(view), std::ranges::end(view)};

    BOOST_CHECK_EQUAL(res, "abcd");
}


BOOST_AUTO_TEST_SUITE_END()


}
