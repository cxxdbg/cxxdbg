// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file test.cpp
/// Contains definition of main function for app test.

#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/included/unit_test.hpp>

#include "cxxdbg/log/log_init.hpp"


bool init_unit_test() {
    auto argc = boost::unit_test::framework::master_test_suite().argc;
    auto argv = boost::unit_test::framework::master_test_suite().argv;
    cxxdbg::log::parse_cmd_line_and_init(argc, argv);
    return true;
}
