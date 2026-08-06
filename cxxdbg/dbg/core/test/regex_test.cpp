// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file regex_test.cpp
/// Contains unit tests for validating regex functions.

#include "../regex.hpp"
#include <boost/test/unit_test.hpp>
#include <iostream>


namespace cxxdbg::dbg::core {


BOOST_AUTO_TEST_SUITE(regex_test)


BOOST_AUTO_TEST_CASE(empty) {
    BOOST_CHECK(validate_regex("") == "regular expression is empty");
}


BOOST_AUTO_TEST_CASE(simple) {
    BOOST_CHECK(validate_regex("foo").empty());
}


BOOST_AUTO_TEST_CASE(parens_bug) {
    BOOST_CHECK(!validate_regex("((").empty());
}


BOOST_AUTO_TEST_SUITE_END()


}
