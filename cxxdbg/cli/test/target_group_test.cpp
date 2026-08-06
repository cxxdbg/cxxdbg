// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file target_group_test.cpp
/// Contains unit tests for target_group class.

#include "mock_exec_processor.hpp"
#include "mock_target_processor.hpp"
#include "cxxdbg/dbg/launch_options.hpp"
#include "cxxdbg/cli/target_group.hpp"
#include "cxxdbg/cli/target_processor.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::cli::test {


struct target_group_test_fixture {
    mock_exec_processor exec_proc;
    mock_target_processor targ_proc;
    target_group grp{targ_proc, exec_proc};
};


BOOST_FIXTURE_TEST_SUITE(target_group_test, target_group_test_fixture)


#define TEST_APP_EXEC_COMMAND_REGISTERED(name, namestr) \
    BOOST_AUTO_TEST_CASE(name##_test) { \
        MOCK_ADD_CALL(exec_proc, exec_cmd, [](auto && cmd, auto && handl) { \
            BOOST_CHECK(cmd == "target " namestr " arg1"); \
        }); \
        \
        grp.exec({namestr, "arg1"}, [](const auto & res) {}); \
        \
        BOOST_CHECK(exec_proc.verify()); \
    }


TEST_APP_EXEC_COMMAND_REGISTERED(list, "list")
TEST_APP_EXEC_COMMAND_REGISTERED(modules, "modules")
TEST_APP_EXEC_COMMAND_REGISTERED(select, "select")
TEST_APP_EXEC_COMMAND_REGISTERED(stop_hook, "stop-hook")
TEST_APP_EXEC_COMMAND_REGISTERED(symbols, "symbols")
TEST_APP_EXEC_COMMAND_REGISTERED(variable, "variable")


/// Tests target create command
BOOST_AUTO_TEST_CASE(test_target_create) {
    MOCK_ADD_CALL(targ_proc, load_target, [](const auto & fname, const auto & work_dir,
                                             const auto & cmd_args, const auto & handler) {
        BOOST_CHECK(fname == "xxx");
        BOOST_CHECK(work_dir == std::filesystem::current_path());
        BOOST_CHECK(cmd_args.empty());
    });

    grp.exec({"create", "xxx"}, [](const auto & res) {});
}


/// Tests target create command with --args
BOOST_AUTO_TEST_CASE(test_target_create_args) {
    MOCK_ADD_CALL(targ_proc, load_target, [](const auto & fname, const auto & work_dir,
                                             const auto & cmd_args, const auto &) {
        BOOST_CHECK(fname == "xxx");
        BOOST_CHECK(work_dir == std::filesystem::current_path());
        BOOST_REQUIRE(cmd_args.size() == 1);
        BOOST_CHECK(cmd_args[0] == "arg1");
    });

    grp.exec({"create", "--args", "xxx", "arg1"}, [](const auto & res) {});
}


/// Tests target delete
BOOST_AUTO_TEST_CASE(test_target_delete) {
    MOCK_ADD_CALL(targ_proc, close_target, [](const auto &) {});

    grp.exec({"delete", "0"}, [](const auto & res) {});
}


BOOST_AUTO_TEST_SUITE_END()


}
