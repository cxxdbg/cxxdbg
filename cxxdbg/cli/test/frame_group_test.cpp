// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file frame_group_test.cpp
/// Contains unit tests for frame_group class.

#include "mock_exec_processor.hpp"
#include "mock_frame_processor.hpp"
#include "cxxdbg/async/result.hpp"
#include "cxxdbg/cli/frame_group.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::cli::test {


BOOST_AUTO_TEST_SUITE(frame_group_test)


#define TEST_APP_EXEC_COMMAND_REGISTERED(name, namestr) \
    BOOST_AUTO_TEST_CASE(name##_test) { \
        mock_exec_processor exec_proc; \
        mock_frame_processor frame_proc; \
        frame_group grp{exec_proc, frame_proc}; \
        \
        MOCK_ADD_CALL(exec_proc, exec_cmd, [](auto && cmd, auto && handl) { \
            BOOST_CHECK(cmd == "frame " namestr " arg1"); \
        }); \
        grp.exec({namestr, "arg1"}, [](const auto & res) {}); \
        \
        BOOST_CHECK(exec_proc.verify()); \
    }


TEST_APP_EXEC_COMMAND_REGISTERED(info, "info")
TEST_APP_EXEC_COMMAND_REGISTERED(variable, "variable")


/// Tests select command
BOOST_AUTO_TEST_CASE(test_select) {
    mock_exec_processor exec_proc;
    mock_frame_processor frame_proc;
    frame_group grp{exec_proc, frame_proc};

    MOCK_ADD_CALL(frame_proc, select_frame, [](auto index, const auto & h) {
        BOOST_CHECK(index == 234);
        h(async::result<>{});
    });

    grp.exec({"select", "234"}, [](const auto & res) {
        BOOST_CHECK(res == "selected frame 234");
    });
}


BOOST_AUTO_TEST_SUITE_END()


}
