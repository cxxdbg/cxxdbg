// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file break_base_group_test.cpp
/// Contains unit tests for the break_base_group class.

#include "mock_break_processor.hpp"
#include "mock_exec_processor.hpp"
#include "../break_base_group.hpp"
#include <boost/test/unit_test.hpp>
#include <iostream>


namespace cxxdbg::dbg::cli::test {


struct break_base_group_test_fixture {
    mock_exec_processor exec_proc;
    mock_break_processor break_proc;
    break_base_group grp{breakpoint_type::watch, "mock_bp", exec_proc, break_proc};
};


#define TEST_APP_EXEC_COMMAND_REGISTERED(name) \
    BOOST_AUTO_TEST_CASE(test_##name) { \
        MOCK_ADD_CALL(exec_proc, exec_cmd, [](auto && cmd, auto && handl) { \
            BOOST_CHECK(cmd == std::string("mock_bp ") + #name + " arg1"); \
        }); \
        grp.exec({#name, "arg1"}, [](const auto & res) {}); \
        BOOST_CHECK(exec_proc.verify()); \
    }


BOOST_FIXTURE_TEST_SUITE(break_base_group_test, break_base_group_test_fixture)


// TODO: implement command command in application classes
//TEST_APP_EXEC_COMMAND_REGISTERED(command)



/// Tests breakpoint delete command
BOOST_AUTO_TEST_CASE(detele_test) {
    MOCK_ADD_CALL(break_proc, delete_breakpoint, [](auto id, auto && h) {
        BOOST_CHECK(id.type() == breakpoint_type::watch);
        BOOST_CHECK(id.num() == 23);
        h(async::result<>{});
    });

    bool hcalled = false;
    grp.exec({"delete", "23"}, [&hcalled](const std::string & res) {
        hcalled = true;
        BOOST_CHECK(res == "deleted mock_bp 23");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests breakpoint enable command
BOOST_AUTO_TEST_CASE(enable_test) {
    MOCK_ADD_CALL(break_proc, enable_breakpoint, [](auto id, auto && h) {
        BOOST_CHECK(id.type() == breakpoint_type::watch);
        BOOST_CHECK(id.num() == 23);
        h(async::result<>{});
    });

    bool hcalled = false;
    grp.exec({"enable", "23"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "enabled mock_bp 23");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests breakpoint disable command
BOOST_AUTO_TEST_CASE(disable_test) {
    MOCK_ADD_CALL(break_proc, disable_breakpoint, [](auto id, auto && h) {
        BOOST_CHECK(id.type() == breakpoint_type::watch);
        BOOST_CHECK(id.num() == 23);
        h(async::result<>{});
    });

    bool hcalled = false;
    grp.exec({"disable", "23"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "disabled mock_bp 23");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests breakpoint modify command with hit count property
BOOST_AUTO_TEST_CASE(modify_hit_count_test) {
    MOCK_ADD_CALL(break_proc, set_breakpoint_props,
    [](auto && id, auto && props, auto && h) {
        BOOST_CHECK(id.type() == breakpoint_type::watch);
        BOOST_CHECK(id.num() == 11);
        BOOST_CHECK(props.enable_hit_count);
        BOOST_CHECK(props.hit_count == 22);
        BOOST_CHECK(props.cond == "");
        h(async::result<>{});
    });

    bool hcalled = false;
    grp.exec({"modify", "11", "-h", "22"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "modified mock_bp 11");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests breakpoint modify command with condition
BOOST_AUTO_TEST_CASE(modify_func_cond_test) {
    MOCK_ADD_CALL(break_proc, set_breakpoint_props,
    [](auto && id, auto && props, auto && h) {
        BOOST_CHECK(id.type() == breakpoint_type::watch);
        BOOST_CHECK(id.num() == 11);
        BOOST_CHECK(!props.enable_hit_count);
        BOOST_CHECK(props.hit_count == 0);
        BOOST_CHECK(props.cond == "ccc");
        h(async::result<>{});
    });

    bool hcalled = false;
    grp.exec({"modify", "11", "-c", "ccc"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "modified mock_bp 11");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests breakpoint modify command with no breakpoint id
BOOST_AUTO_TEST_CASE(modify_no_id_test) {
    try {
        grp.exec({"modify", "-c", "aaaa"}, [](const auto & res) {});
        BOOST_CHECK(false);
    }
    catch(std::exception & ex) {
        // program options reported error
    }
}


/// Tests list command
BOOST_AUTO_TEST_CASE(list_test) {
    MOCK_ADD_CALL(break_proc, list_breakpoints, [](auto type) {
        BOOST_CHECK(type == breakpoint_type::watch);
        return "bp list!";
    });

    bool hcalled = false;
    grp.exec({"list"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "bp list!");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests help command
BOOST_AUTO_TEST_CASE(help) {
    bool called = false;
    grp.exec({}, [&called](const auto & res) {
        called = true;
        BOOST_CHECK_EQUAL(res,
                          "Supported commands:\n"
                          "\n"
                          "    delete  -- Delete mock_bp\n"
                          "    disable -- Disable mock_bp\n"
                          "    enable  -- Enable mock_bp\n"
                          "    list    -- List all mock_bps\n"
                          "    modify  -- Modify mock_bp\n");
    });

    BOOST_CHECK(called);
}


BOOST_AUTO_TEST_SUITE_END()

}
