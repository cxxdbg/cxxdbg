// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file command_group_test.cpp
/// Contains unit tests for the command_group class.

#include "cxxdbg/cli/function_command.hpp"
#include "cxxdbg/cli/command_group.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::cli::test {


BOOST_AUTO_TEST_SUITE(command_group_test)


/// Tests calling execute on empty group
BOOST_AUTO_TEST_CASE(execute_empty_group) {
    command_group grp{"desc"};

    bool err_handled = false;

    try {
        grp.exec({"my command"}, [](const auto &) {
            BOOST_CHECK(false);
        });
        BOOST_CHECK(false);
    }
    catch(std::runtime_error & err) {
        err_handled = true;
        BOOST_CHECK(std::string("command 'my command' not found") == err.what());
    }

    BOOST_CHECK(err_handled);
}


/// Tests executing simple command
BOOST_AUTO_TEST_CASE(execute_simple) {

    // registering commands

    command_group grp{"desc"};

    bool cmd2_called = false;

    grp.reg_func_cmd("cmd1", "cmd1d", "cmd1 help", [](const auto & args, const auto & chandl) {
        BOOST_CHECK(false);
    });

    grp.reg_func_cmd("cmd2", "cmd2d", "cmd2 help", [&cmd2_called](const auto & args, const auto & chandl) {
        cmd2_called = true;
        BOOST_REQUIRE(args.size() == 1);
        BOOST_CHECK(args[0] == "some-arg");
        chandl("my result");
    });


    // executing command

    bool chandler_called = false;

    grp.exec({"cmd2", "some-arg"}, [&chandler_called](const auto & res) {
        chandler_called = true;
        BOOST_CHECK(res == "my result");
    });

    BOOST_CHECK(cmd2_called);
    BOOST_CHECK(chandler_called);
}


/// Tests throwing exception in command
BOOST_AUTO_TEST_CASE(execute_throw) {

    // registering commands

    command_group grp{"desc"};

    bool cmd2_called = false;

    grp.reg_func_cmd("cmd1", "cmd1d", "cmd1 help", [](const auto & args, const auto & chandl) {
        BOOST_CHECK(false);
    });

    grp.reg_func_cmd("cmd2", "cmd2d", "cmd2 help", [&cmd2_called](const auto & args, const auto & chandl) {
        cmd2_called = true;
        BOOST_REQUIRE(args.size() == 1);
        BOOST_CHECK(args[0] == "some-arg");
        throw std::runtime_error("my error");
    });


    // executing command

    bool err_handled = false;

    try {
        grp.exec({"cmd2", "some-arg"}, [](const auto &) {
            BOOST_CHECK(false);
        });
        BOOST_CHECK(false);
    }
    catch(std::exception & ex) {
        err_handled = true;
        BOOST_CHECK(std::string("cmd2: my error") == ex.what());
    }

    BOOST_CHECK(cmd2_called);
    BOOST_CHECK(err_handled);
}


/// Tests displaying help for subcommand
BOOST_AUTO_TEST_CASE(subcommand_help) {

    // registering commands

    command_group grp{"desc"};

    grp.reg_func_cmd("cmd1", "cmd1d", "cmd1 help", [](const auto & ... args) {
        BOOST_CHECK(false);
    });

    grp.reg_func_cmd("cmd2", "cmd2d", "cmd2 help", [](const auto & ... args) {
        BOOST_CHECK(false);
    });


    // displaying help message for cmd1

    bool handler_called = false;

    grp.help({"cmd1"}, [&handler_called](const auto & res) {
        handler_called = true;
        BOOST_CHECK(res == "cmd1 help");
    });

    BOOST_CHECK(handler_called);
}


/// Tests throwing exception when displaying help for subcommand
BOOST_AUTO_TEST_CASE(subcommand_help_throw) {

    // registering commands

    command_group grp{"desc"};

    grp.reg_func_cmd("cmd1", "cmd1d", "cmd1 help", [](const auto & ... args) {
        BOOST_CHECK(false);
    });

    grp.reg_func_cmd("cmd2", "cmd2d", "cmd2 help", [](const auto & ... args) {
        BOOST_CHECK(false);
    });


    // displaying help message

    bool ex_caught = false;

    try {
        grp.help({"cmdx"}, [](const auto & res) {
            BOOST_CHECK(false);
        });
    }
    catch (std::exception & ex) {
        ex_caught = true;
        BOOST_CHECK(std::string("command 'cmdx' not found") == ex.what());
    }

    BOOST_CHECK(ex_caught);
}


/// Tests displaying help for group
BOOST_AUTO_TEST_CASE(group_help) {

    // registering commands

    command_group grp{"desc"};

    grp.reg_func_cmd("cmd1", "cmd1d", "cmd1 help", [](const auto & ... args) {
        BOOST_CHECK(false);
    });

    grp.reg_func_cmd("cmd2 long long", "cmd2d", "cmd2 help", [](const auto & ... args) {
        BOOST_CHECK(false);
    });


    // displaying help message

    bool handler_called = false;

    grp.help({}, [&handler_called](const auto & res) {
        handler_called = true;
        BOOST_CHECK(res == "Supported commands:\n"
                           "\n"
                           "    cmd1           -- cmd1d\n"
                           "    cmd2 long long -- cmd2d\n");
    });

    BOOST_CHECK(handler_called);
}


/// Tests displaying help for group after executing command with no args
BOOST_AUTO_TEST_CASE(group_help_no_cmd) {

    // registering commands

    command_group grp{"desc"};

    grp.reg_func_cmd("cmd1", "cmd1d", "cmd1 help", [](const auto & ... args) {
        BOOST_CHECK(false);
    });

    grp.reg_func_cmd("cmd2 long long", "cmd2d", "cmd2 help", [](const auto & ... args) {
        BOOST_CHECK(false);
    });


    // displaying help message

    bool handler_called = false;

    grp.exec({}, [&handler_called](const auto & res) {
        handler_called = true;
        BOOST_CHECK(res == "Supported commands:\n"
                           "\n"
                           "    cmd1           -- cmd1d\n"
                           "    cmd2 long long -- cmd2d\n");
    });

    BOOST_CHECK(handler_called);
}



BOOST_AUTO_TEST_SUITE_END()


}
