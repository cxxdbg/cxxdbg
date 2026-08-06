// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file join_delim_test.cpp
/// Contain unit tests for join_delim view for ranges.

#include <ranges.hpp>
#include <boost/test/unit_test.hpp>
#include <ranges.hpp>


namespace xranges::test {


BOOST_AUTO_TEST_SUITE(join_delim_test)


/// Tests empty view
BOOST_AUTO_TEST_CASE(empty) {
    std::vector<std::string> v;
    auto res = v | views::join_delim('x');
    BOOST_CHECK(std::ranges::begin(res) == std::ranges::end(res));
}


/// Tests iteration over range
BOOST_AUTO_TEST_CASE(iterate) {
    std::vector<std::string> v;
    auto res = v | views::join_delim('x');

    v.push_back("ab");
    v.push_back("");
    v.push_back("cd");

    auto it = std::ranges::begin(res);
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK(*it == 'a');

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK(*it == 'b');

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK(*it == 'x');

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK(*it == 'x');

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK(*it == 'c');

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK(*it == 'd');

    ++it;
    BOOST_CHECK(it == std::ranges::end(res));
}


/// Tests join_delim adaptor with transform view on vector of strings
BOOST_AUTO_TEST_CASE(tramsform_const_string_vector) {
    std::vector<std::string> v;
    v.push_back("ab");
    v.push_back("");
    v.push_back("cd");

    auto fn = [](std::string & s) -> const std::string & { return const_cast<const std::string &>(s); };
    auto res = v | std::ranges::views::transform(fn) | views::join_delim('x');

    auto it = std::ranges::begin(res);
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK(*it == 'a');

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK(*it == 'b');

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK(*it == 'x');

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK(*it == 'x');

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK(*it == 'c');

    ++it;
    BOOST_REQUIRE(it != std::ranges::end(res));
    BOOST_CHECK(*it == 'd');

    ++it;
    BOOST_CHECK(it == std::ranges::end(res));
}


/// Tests constructing string from view iterators
BOOST_AUTO_TEST_CASE(test_string_construct) {
    std::vector<std::string> v;
    v.push_back("ab");
    v.push_back("cd");

    auto view = v | views::join_delim('x');
    std::string res{std::ranges::begin(view), std::ranges::end(view)};
    BOOST_CHECK_EQUAL(res, "abxcd");
}


BOOST_AUTO_TEST_SUITE_END()


}
