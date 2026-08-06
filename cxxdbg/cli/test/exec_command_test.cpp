// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file exec_command_test.cpp
/// Contains unit tests for exec_command class.

#include "mock_exec_processor.hpp"
#include "cxxdbg/cli/exec_command.hpp"
#include "cxxdbg/cli/exec_processor.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::cli::test {


BOOST_AUTO_TEST_SUITE(exec_command_test)


/// Simple test for app_exec_command_handler::exec
BOOST_AUTO_TEST_CASE(test_exec) {
    mock_exec_processor proc;
    exec_command handl{proc, "baba bobo", "desc"};

    MOCK_ADD_CALL(proc, exec_cmd, [](auto && cmd, auto && handl) {
        BOOST_CHECK(cmd == "baba bobo my command");
        handl("test result");
    });

    bool handler_called = false;
    handl.exec({"my", "command"}, [&handler_called](const auto & res) {
        handler_called = true;
        BOOST_CHECK(res == "test result");
    });

    BOOST_CHECK(proc.verify());
    BOOST_CHECK(handler_called);
}


/// Simple test for app_exec_command_handler::help
BOOST_AUTO_TEST_CASE(test_help) {
    mock_exec_processor proc;
    exec_command handl{proc, "baba bobo", "desc"};

    MOCK_ADD_CALL(proc, exec_cmd, [](auto && cmd, auto && handl) {
        BOOST_CHECK(cmd == "help baba bobo my command");
        handl("test result");
    });

    bool handler_called = false;
    handl.help({"my", "command"}, [&handler_called](const auto & res) {
        handler_called = true;
        BOOST_CHECK(res == "test result");
    });

    BOOST_CHECK(handler_called);
    BOOST_CHECK(proc.verify());
}


/// Displaying help with no args
BOOST_AUTO_TEST_CASE(test_help_noargs) {
    mock_exec_processor proc;
    exec_command handl{proc, "baba bobo", "desc"};

    MOCK_ADD_CALL(proc, exec_cmd, [](auto && cmd, auto && handl) {
        BOOST_CHECK(cmd == "help baba bobo");
        handl("test result");
    });

    bool handler_called = false;
    handl.help({}, [&handler_called](const auto & res) {
        handler_called = true;
        BOOST_CHECK(res == "test result");
    });

    BOOST_CHECK(handler_called);
    BOOST_CHECK(proc.verify());
}


/// Tests executing command with no arguments
/// Simple test for app_exec_command_handler::exec
BOOST_AUTO_TEST_CASE(test_exec_no_args) {
    mock_exec_processor proc;
    exec_command handl{proc, "baba bobo", "desc"};

    MOCK_ADD_CALL(proc, exec_cmd, [](auto && cmd, auto && handl) {
        BOOST_CHECK_EQUAL(cmd, "baba bobo");
        handl("test result");
    });

    bool handler_called = false;
    handl.exec({}, [&handler_called](const auto & res) {
        handler_called = true;
        BOOST_CHECK(res == "test result");
    });

    BOOST_CHECK(proc.verify());
    BOOST_CHECK(handler_called);
}



BOOST_AUTO_TEST_SUITE_END()


}
