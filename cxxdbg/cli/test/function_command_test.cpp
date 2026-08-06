// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file function_command_test.
/// Contains unit tests for function_command class.

#include "cxxdbg/cli/function_command.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::cli::test {


BOOST_AUTO_TEST_SUITE(function_command_test)


/// Simple test for function_command_handler
BOOST_AUTO_TEST_CASE(test_simple) {

    bool func_called = false;

    function_command handl("desc", "help", [&func_called](const auto & cmd, const auto & chandl) {
        func_called = true;

        BOOST_REQUIRE(cmd.size() == 2);
        BOOST_CHECK(cmd[0] == "aaaa");
        BOOST_CHECK(cmd[1] == "bbb");

        chandl("result");
    });

    bool chandler_called = false;
    bool hhandler_called = false;

    handl.exec({"aaaa", "bbb"}, [&chandler_called](const auto & res) {
        chandler_called = true;
        BOOST_CHECK(res == "result");
    });

    handl.help({}, [&hhandler_called](const auto & res) {
        hhandler_called = true;
        BOOST_CHECK(res == "help");
    });

    BOOST_CHECK(func_called);
    BOOST_CHECK(chandler_called);
    BOOST_CHECK(hhandler_called);
}


/// Tests throwing exception in help message
BOOST_AUTO_TEST_CASE(test_throw_help) {

    function_command handl("help", "desc", [](const auto & ... args) {});

    bool ex_handled = false;

    try {
        handl.help({"arg"}, [](const auto &) {
            BOOST_CHECK(false);
        });
        BOOST_CHECK(false);
    }
    catch(std::exception & ex) {
        ex_handled = true;
        BOOST_CHECK(std::string("command 'arg' not found") == ex.what());
    }

    BOOST_CHECK(ex_handled);
}


/// Tests displaying help message
BOOST_AUTO_TEST_CASE(test_help_handler) {

    function_command cmd{"desc", "help msg", [](const auto & ... args) {}};
    cmd.help({}, [](const auto & res) {
        BOOST_CHECK(res == "help msg");
    });
}



BOOST_AUTO_TEST_SUITE_END()


}
