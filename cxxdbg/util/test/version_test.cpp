// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file version_test.cpp
/// Contains unit tests for functions for working with version strings.

#include "../version.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::util::test {


BOOST_AUTO_TEST_SUITE(version_test)


/// Checks == comparison operator with equal versions
BOOST_AUTO_TEST_CASE(compare_equal_true) {
    BOOST_CHECK((version{10, 20, 30} == version{10, 20, 30}));
}


/// Checks == comparison operator with different versions
BOOST_AUTO_TEST_CASE(compare_equal_false) {
    BOOST_CHECK((version{10, 20, 30} != version{10, 20, 31}));
}


BOOST_AUTO_TEST_CASE(compare_less_1) {
    BOOST_CHECK((version{10, 20, 30} < version{10, 20, 31}));
}

BOOST_AUTO_TEST_CASE(compare_less_2) {
    BOOST_CHECK(!(version{10, 20, 31} < version{10, 20, 31}));
}

BOOST_AUTO_TEST_CASE(compare_less_3) {
    BOOST_CHECK(!(version{10, 20, 32} < version{10, 20, 31}));
}

BOOST_AUTO_TEST_CASE(compare_less_4) {
    BOOST_CHECK((version{10, 20, 30} < version{10, 21, 30}));
}

BOOST_AUTO_TEST_CASE(compare_less_5) {
    BOOST_CHECK((version{10, 20, 36} < version{10, 21, 30}));
}

BOOST_AUTO_TEST_CASE(compare_less_6) {
    BOOST_CHECK(!(version{11, 20, 36} < version{10, 21, 30}));
}

BOOST_AUTO_TEST_CASE(compare_less_7) {
    BOOST_CHECK((version{11, 30, 36} < version{12, 21, 30}));
}


/// Tests parsing version from string
BOOST_AUTO_TEST_CASE(parse) {
    auto v = parse_version("0.15.2");
    BOOST_CHECK_EQUAL(v, (version{0, 15, 2}));
}


/// Tests parsing invalid version from string
BOOST_AUTO_TEST_CASE(parse_invalid) {
    auto v = parse_version("15.2.exe");
    BOOST_CHECK(v.is_zero());
}


/// Tests parsing invalid version from string
BOOST_AUTO_TEST_CASE(parse_invalid_1) {
    auto v = parse_version("15.2.12.11");
    BOOST_CHECK(v.is_zero());
}


BOOST_AUTO_TEST_SUITE_END()


}
