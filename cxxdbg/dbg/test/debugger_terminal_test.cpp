// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file debugger_terminal_test.cpp
/// Contains unit test for the debugger_terminal class.

#include "cxxdbg/app/memory_settings_storage.hpp"
#include "cxxdbg/dbg/debugger_terminal.hpp"
#include "cxxdbg/cli/app_command_interpreter.hpp"
#include "cxxdbg/cli/test/mock_code_break_processor.hpp"
#include "cxxdbg/cli/test/mock_exec_processor.hpp"
#include "cxxdbg/cli/test/mock_frame_processor.hpp"
#include "cxxdbg/cli/test/mock_platform_processor.hpp"
#include "cxxdbg/cli/test/mock_process_processor.hpp"
#include "cxxdbg/cli/test/mock_target_processor.hpp"
#include "cxxdbg/cli/test/mock_thread_processor.hpp"
#include "cxxdbg/cli/test/mock_watch_processor.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::test {


struct debugger_terminal_test_fixture {
    cli::test::mock_code_break_processor break_proc;
    cli::test::mock_exec_processor exec_proc;
    cli::test::mock_frame_processor frame_proc;
    cli::test::mock_process_processor proc_proc;
    cli::test::mock_target_processor targ_proc;
    cli::test::mock_thread_processor thread_proc;
    cli::test::mock_watch_processor watch_proc;
    cli::test::mock_platform_processor platform_proc;

    cli::app_command_interpreter interp{
        break_proc, exec_proc, frame_proc, proc_proc, targ_proc, thread_proc, watch_proc, platform_proc};
};


BOOST_FIXTURE_TEST_SUITE(debugger_terminal_test, debugger_terminal_test_fixture)


/// Tests reading empty command history
BOOST_AUTO_TEST_CASE(read_hist_empty) {
    memory_settings_storage sett;
    debugger_terminal term{sett, interp, 2};

    BOOST_CHECK(term.next_cmd() == "");
    BOOST_CHECK(term.prev_cmd() == "");
}


/// Tests executing command
BOOST_AUTO_TEST_CASE(test_exec) {
    memory_settings_storage sett;
    debugger_terminal term{sett, interp};

    MOCK_ADD_CALL(exec_proc, exec_cmd, [](auto && cmd, auto && handl) {
        BOOST_CHECK(cmd == "help memory");
        handl("xxx");
    });

    term.exec("help memory", [](auto && res) {
        BOOST_CHECK(res == "xxx");
    });

    BOOST_CHECK(exec_proc.verify());
}


/// Tests reading history from settings
BOOST_AUTO_TEST_CASE(read_hist) {
    memory_settings_storage sett;
    std::vector<std::string> cmds{"aaaa", "bbbb"};
    sett.write("debugger_terminal/history", cmds);

    debugger_terminal term{sett, interp, 4};
    BOOST_CHECK(term.next_cmd() == "");
    BOOST_CHECK(term.prev_cmd() == "bbbb");
    BOOST_CHECK(term.prev_cmd() == "aaaa");
    BOOST_CHECK(term.prev_cmd() == "");
    BOOST_CHECK(term.next_cmd() == "bbbb");
    BOOST_CHECK(term.next_cmd() == "");
}


/// Tests reading long history from settings
BOOST_AUTO_TEST_CASE(read_hist_long) {
    memory_settings_storage sett;
    std::vector<std::string> cmds{"aaaa", "bbbb", "ccc"};
    sett.write("debugger_terminal/history", cmds);

    debugger_terminal term{sett, interp, 2};
    BOOST_CHECK(term.next_cmd() == "");
    BOOST_CHECK(term.prev_cmd() == "ccc");
    BOOST_CHECK(term.prev_cmd() == "bbbb");
    BOOST_CHECK(term.prev_cmd() == "");
    BOOST_CHECK(term.next_cmd() == "ccc");
    BOOST_CHECK(term.next_cmd() == "");
}


/// Tests adding command to history
BOOST_AUTO_TEST_CASE(exec_add_hist) {
    memory_settings_storage sett;
    debugger_terminal term{sett, interp};

    term.exec("help memory a", [](auto && res) {});

    BOOST_CHECK(term.prev_cmd() == "help memory a");
    BOOST_CHECK(term.prev_cmd() == "");
}


/// Tests adding command to history with history overflow
BOOST_AUTO_TEST_CASE(exec_add_hist_overflow) {
    memory_settings_storage sett;
    debugger_terminal term{sett, interp, 2};

    term.exec("help memory a", [](auto && res) {});
    term.exec("help memory b", [](auto && res) {});
    term.exec("help memory c", [](auto && res) {});

    BOOST_CHECK(term.prev_cmd() == "help memory c");
    BOOST_CHECK(term.prev_cmd() == "help memory b");
    BOOST_CHECK(term.prev_cmd() == "");
}



/// Tests saving history to settings after exec
BOOST_AUTO_TEST_CASE(exec_save_hist) {
    memory_settings_storage sett;
    std::vector<std::string> cmds{"aaaa", "bbbb"};
    sett.write("debugger_terminal/history", cmds);
    debugger_terminal term{sett, interp};

    term.exec("help memory a", [](auto && res) {});

    std::vector<std::string> hist = sett.read<std::vector<std::string>>("debugger_terminal/history", {});
    BOOST_REQUIRE(hist.size() == 3);
    BOOST_CHECK(hist[0] == "aaaa");
    BOOST_CHECK(hist[1] == "bbbb");
    BOOST_CHECK(hist[2] == "help memory a");
}


/// Tests saving history to settings after exec with overflow
BOOST_AUTO_TEST_CASE(exec_save_hist_overflow) {
    memory_settings_storage sett;
    std::vector<std::string> cmds{"aaaa", "bbbb"};
    sett.write("debugger_terminal/history", cmds);
    debugger_terminal term{sett, interp, 2};

    term.exec("help memory a", [](auto && res) {});

    std::vector<std::string> hist = sett.read<std::vector<std::string>>("debugger_terminal/history", {});
    BOOST_REQUIRE(hist.size() == 2);
    BOOST_CHECK(hist[0] == "bbbb");
    BOOST_CHECK(hist[1] == "help memory a");
}


BOOST_AUTO_TEST_SUITE_END()


}
