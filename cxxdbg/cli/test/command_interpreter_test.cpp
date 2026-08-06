// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file command_interpreter_test.cpp
/// Contains unit tests for the command_interpreter class.

#include "../command_group.hpp"
#include "cxxdbg/cli/command_interpreter.hpp"
#include "cxxdbg/cli/function_command.hpp"
#include "cxxdbg/mock/mock.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::cli::test {


class mock_command:
        public command,
        public mock::object<mock_command> {
public:
    MOCK_DEFINE_METHOD_2_C(exec,
                           void (const string_vector & cmd,
                                 const completion_handler & chandler))
    MOCK_DEFINE_METHOD_2_C(help,
                           void (const string_vector & cmd,
                                 const completion_handler & handler))
};


BOOST_AUTO_TEST_SUITE(command_interpreter_test)


/// Tests executing command handler
BOOST_AUTO_TEST_CASE(test_exec) {
    command_interpreter interp;

    bool handler_called = false;

    std::shared_ptr<mock_command> cmd(new mock_command);
    cmd->set_desc("desc");
    MOCK_ADD_CALL(*cmd, exec, [&handler_called](const auto & cmd, const auto & chandler) {
        handler_called = true;
        BOOST_REQUIRE(cmd.size() == 2);
        BOOST_CHECK(cmd[0] == "arg1");
        BOOST_CHECK(cmd[1] == "arg2");
    });

    interp.reg_cmd("mycmd", cmd);
    interp.exec("  mycmd arg1 arg2", [](const auto &) {});

    BOOST_CHECK(handler_called);
}


/// Tests displaying help
BOOST_AUTO_TEST_CASE(test_cmd_help) {
    command_interpreter interp;

    bool handler_called = false;

    std::shared_ptr<mock_command> cmd{new mock_command};
    cmd->set_desc("desc");
    MOCK_ADD_CALL(*cmd, help, [&handler_called](const auto & cmd, const auto & chandler) {
        handler_called = true;
        BOOST_REQUIRE(cmd.size() == 2);
        BOOST_CHECK(cmd[0] == "arg1");
        BOOST_CHECK(cmd[1] == "arg2");
    });

    interp.reg_cmd("mycmd2", cmd);

    interp.reg_alias("myalias", "mycmd2");

    interp.exec("  help  mycmd2 arg1 arg2", [](const auto &) {});

    BOOST_CHECK(handler_called);
}



/// Tests aliases
BOOST_AUTO_TEST_CASE(test_alias) {
    command_interpreter interp;

    bool handler_called = false;

    std::shared_ptr<mock_command> cmd(new mock_command);
    cmd->set_desc("desc");
    MOCK_ADD_CALL(*cmd, exec, [&handler_called](const auto & cmd, const auto & chandler) {
        handler_called = true;
        BOOST_REQUIRE(cmd.size() == 3);
        BOOST_CHECK(cmd[0] == "arg1");
        BOOST_CHECK(cmd[1] == "arg2");
        BOOST_CHECK(cmd[2] == "23");
    });

    interp.reg_cmd("mycmd", cmd);
    interp.reg_alias("aaa", "mycmd arg1 arg2");
    interp.exec("aaa 23", [](const auto & res) {});

    BOOST_CHECK(handler_called);
}


/// Tests aliases help
BOOST_AUTO_TEST_CASE(test_alias_help) {
    command_interpreter interp;

    std::shared_ptr<mock_command> cmd(new mock_command);
    cmd->set_desc("desc");
    MOCK_ADD_CALL(*cmd, help, [](const auto & cmd, const auto & chandler) {
        BOOST_REQUIRE(cmd.size() == 2);
        BOOST_CHECK(cmd[0] == "arg1");
        BOOST_CHECK(cmd[1] == "arg2");
        chandler("HELP RES");
    });

    interp.reg_cmd("mycmd", cmd);
    interp.reg_alias("aaa", "mycmd arg1 arg2");

    bool handler_called = false;
    interp.exec("help aaa", [&handler_called](const auto & res) {
        BOOST_CHECK(res == "HELP RES");
        handler_called = true;
    });

    BOOST_CHECK(handler_called);
    BOOST_CHECK(cmd->verify());
}


/// Tests displaying help for all commands
BOOST_AUTO_TEST_CASE(test_help) {
    command_interpreter interp;

    auto grp = std::make_shared<command_group>("group desc");
    interp.reg_cmd("mygroup", grp);

    auto cmd = std::make_shared<mock_command>();
    cmd->set_desc("command desc");
    grp->reg_cmd("mycmd2", cmd);

    interp.reg_alias("myalias", "mygroup mycmd2");

    auto cmd3 = std::make_shared<mock_command>();
    cmd3->set_desc("command3 desc");
    interp.reg_cmd("xxx", cmd3);

    interp.reg_alias("myalias2", "xxx");

    bool handler_called = false;
    interp.exec("help", [&handler_called](const auto & res) {

        const char * exp_res =
                "Supported commands:\n"
                "\n"
                "    mygroup -- group desc\n"
                "    xxx     -- command3 desc\n"
                "\n"
                "Command aliases:\n"
                "\n"
                "    myalias  -- command desc\n"
                "    myalias2 -- command3 desc\n";

        BOOST_CHECK(res == exp_res);

        handler_called = true;
    });

    BOOST_CHECK(handler_called);
}


BOOST_AUTO_TEST_SUITE_END()


}
