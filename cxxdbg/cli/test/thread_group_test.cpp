// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread_group_test.cpp
/// Contains unit tests for thread_group class.

#include "mock_exec_processor.hpp"
#include "mock_thread_processor.hpp"
#include "cxxdbg/async/result.hpp"
#include "cxxdbg/cli/thread_group.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::cli::test {


struct thread_group_test_fixture {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};
};


BOOST_FIXTURE_TEST_SUITE(thread_group_test, thread_group_test_fixture)


#define TEST_APP_EXEC_COMMAND_REGISTERED(name) \
    BOOST_AUTO_TEST_CASE(test_##name) { \
        MOCK_ADD_CALL(exec_proc, exec_cmd, [](auto && cmd, auto && handl) { \
            BOOST_CHECK(cmd == std::string("thread ") + #name + " arg1"); \
        }); \
        \
        grp.exec({#name, "arg1"}, [](const auto & res) {}); \
        \
        BOOST_CHECK(exec_proc.verify()); \
    }


TEST_APP_EXEC_COMMAND_REGISTERED(backtrace)
TEST_APP_EXEC_COMMAND_REGISTERED(info)
TEST_APP_EXEC_COMMAND_REGISTERED(jump)
TEST_APP_EXEC_COMMAND_REGISTERED(list)


/// Tests select command
BOOST_AUTO_TEST_CASE(test_select) {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};

    MOCK_ADD_CALL(thread_proc, select_thread, [](const auto index, const auto & h) {
        BOOST_CHECK(index == 238);
        h(async::result<>{});
    });

    grp.exec({"select", "238"}, [](const auto & res) {
        BOOST_CHECK(res == "selected thread 238");
    });
}


/// Tests step into with no options
BOOST_AUTO_TEST_CASE(test_step_into) {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};

    bool handler_called = false;
    MOCK_ADD_CALL(thread_proc, step_into, [&handler_called](auto avoid_nodebug,
                                                            auto target_func,
                                                            auto avoid_regex,
                                                            auto step_through_regex,
                                                            const auto & h) {
        BOOST_CHECK(boost::indeterminate(avoid_nodebug));
        BOOST_CHECK(target_func.empty());
        BOOST_CHECK(avoid_regex.empty());
        BOOST_CHECK(step_through_regex.empty());
        handler_called = true;
        h(async::result<>{});
    });

    grp.exec({"step-in"}, [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(handler_called);
}


/// Tests step into command with -a option
BOOST_AUTO_TEST_CASE(test_step_into_a) {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};

    bool handler_called = false;
    MOCK_ADD_CALL(thread_proc, step_into, [&handler_called](auto avoid_nodebug,
                                                            auto target_func,
                                                            auto avoid_regex,
                                                            auto step_through_regex,
                                                            const auto & h) {
        BOOST_CHECK(static_cast<bool>(avoid_nodebug));
        BOOST_CHECK(target_func.empty());
        BOOST_CHECK(avoid_regex.empty());
        BOOST_CHECK(step_through_regex.empty());
        handler_called = true;
        h(async::result<>{});
    });

    grp.exec({"step-in", "-a", "true"}, [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(handler_called);
}


/// Tests step into command with -A option
BOOST_AUTO_TEST_CASE(test_step_into_A) {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};

    bool handler_called = false;
    MOCK_ADD_CALL(thread_proc, step_into, [&handler_called](auto avoid_nodebug,
                                                            auto target_func,
                                                            auto avoid_regex,
                                                            auto step_through_regex,
                                                            const auto & h) {
        BOOST_CHECK(!static_cast<bool>(avoid_nodebug));
        BOOST_CHECK(target_func.empty());
        BOOST_CHECK(avoid_regex.empty());
        BOOST_CHECK(step_through_regex.empty());
        handler_called = true;
        h(async::result<>{});
    });

    grp.exec({"step-in", "-A", "false"}, [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(handler_called);
}


/// Tests step into command with -A -a options
BOOST_AUTO_TEST_CASE(test_step_into_Aa) {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};

    bool handler_called = false;
    MOCK_ADD_CALL(thread_proc, step_into, [&handler_called](auto avoid_nodebug,
                                                            auto target_func,
                                                            auto avoid_regex,
                                                            auto step_through_regex,
                                                            const auto & h) {
        BOOST_CHECK(!static_cast<bool>(avoid_nodebug));
        BOOST_CHECK(target_func.empty());
        BOOST_CHECK(avoid_regex.empty());
        BOOST_CHECK(step_through_regex.empty());
        handler_called = true;
        h(async::result<>{});
    });

    grp.exec({"step-in", "-A", "false", "-a", "false"}, [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(handler_called);
}


/// Tests step into command with different values of -A -a options
BOOST_AUTO_TEST_CASE(test_step_into_Aa_diff) {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};

    MOCK_ADD_CALL(thread_proc, step_into, [](auto avoid_nodebug,
                                             auto target_func,
                                             auto avoid_regex,
                                             auto step_through_regex,
                                             const auto & h) {
        BOOST_CHECK(false);
    });

    try {
        grp.exec({"step-in", "-A", "false", "-a", "true"}, [](const auto & res) {
            BOOST_CHECK(false);
        });
        BOOST_CHECK(false);
    }
    catch(std::exception & ex) {
        BOOST_CHECK(ex.what() ==
                    std::string{"step-in: different values for -A and -a options not supported"});
    }
}


/// Tests step into command with -t option
BOOST_AUTO_TEST_CASE(test_step_into_target) {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};

    bool handler_called = false;
    MOCK_ADD_CALL(thread_proc, step_into, [&handler_called](auto avoid_nodebug,
                                                            auto target_func,
                                                            auto avoid_regex,
                                                            auto step_through_regex,
                                                            const auto & h) {
        BOOST_CHECK(boost::indeterminate(avoid_nodebug));
        BOOST_CHECK(target_func == "foo");
        BOOST_CHECK(avoid_regex.empty());
        BOOST_CHECK(step_through_regex.empty());
        handler_called = true;
        h(async::result<>{});
    });

    grp.exec({"step-in", "-t", "foo"}, [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(handler_called);
}


/// Tests step into command with -r option
BOOST_AUTO_TEST_CASE(test_step_into_avoid_regex) {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};

    bool handler_called = false;
    MOCK_ADD_CALL(thread_proc, step_into, [&handler_called](auto avoid_nodebug,
                                                            auto target_func,
                                                            auto avoid_regex,
                                                            auto step_through_regex,
                                                            const auto & h) {
        BOOST_CHECK(boost::indeterminate(avoid_nodebug));
        BOOST_CHECK(target_func.empty());
        BOOST_CHECK(avoid_regex == "foo");
        BOOST_CHECK(step_through_regex.empty());
        handler_called = true;
        h(async::result<>{});
    });

    grp.exec({"step-in", "-r", "foo"}, [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(handler_called);
}


/// Tests step into command with -s option
BOOST_AUTO_TEST_CASE(test_step_into_step_through_regex) {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};

    bool handler_called = false;
    MOCK_ADD_CALL(thread_proc, step_into, [&handler_called](auto avoid_nodebug,
                                                            auto target_func,
                                                            auto avoid_regex,
                                                            auto step_through_regex,
                                                            const auto & h) {
        BOOST_CHECK(boost::indeterminate(avoid_nodebug));
        BOOST_CHECK(target_func.empty());
        BOOST_CHECK(avoid_regex.empty());
        BOOST_CHECK(step_through_regex == "^std::function");
        handler_called = true;
        h(async::result<>{});
    });

    grp.exec({"step-in", "-s", "^std::function"}, [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(handler_called);
}


/// Tests step over command
BOOST_AUTO_TEST_CASE(test_step_over) {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};

    bool handler_called = false;
    MOCK_ADD_CALL(thread_proc, step_over, [&handler_called](auto step_through_regex,
                                                            const auto & h) {
        handler_called = true;
        BOOST_CHECK(step_through_regex.empty());
        h(async::result<>{});
    });

    grp.exec({"step-over"}, [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(handler_called);
}


/// Tests step over command with -s option
BOOST_AUTO_TEST_CASE(test_step_over_step_through) {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};

    bool handler_called = false;
    MOCK_ADD_CALL(thread_proc, step_over, [&handler_called](auto step_through_regex,
                                                            const auto & h) {
        handler_called = true;
        BOOST_CHECK(step_through_regex == "myfoo");
        h(async::result<>{});
    });

    grp.exec({"step-over", "-s", "myfoo"}, [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(handler_called);
}


/// Tests step out command
BOOST_AUTO_TEST_CASE(test_step_out) {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};

    bool handler_called = false;
    MOCK_ADD_CALL(thread_proc, step_out, [&handler_called](auto avoid_nodebug,
                                                           auto step_through_regex,
                                                           const auto & h) {
        BOOST_CHECK(boost::indeterminate(avoid_nodebug));
        BOOST_CHECK(step_through_regex.empty());
        handler_called = true;
        h(async::result<>{});
    });

    grp.exec({"step-out"}, [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(handler_called);
}


/// Tests step out command with avoid nodebug -a option
BOOST_AUTO_TEST_CASE(test_step_out_avoid_nodebug_a) {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};

    bool handler_called = false;
    MOCK_ADD_CALL(thread_proc, step_out, [&handler_called](auto avoid_nodebug,
                                                           auto step_through_regex,
                                                           const auto & h) {
        BOOST_CHECK(static_cast<bool>(avoid_nodebug));
        BOOST_CHECK(step_through_regex.empty());
        handler_called = true;
        h(async::result<>{});
    });

    grp.exec({"step-out", "-a", "true"}, [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(handler_called);
}


/// Tests step out command with avoid nodebug -A option
BOOST_AUTO_TEST_CASE(test_step_out_avoid_nodebug_A) {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};

    bool handler_called = false;
    MOCK_ADD_CALL(thread_proc, step_out, [&handler_called](auto avoid_nodebug,
                                                           auto step_through_regex,
                                                           const auto & h) {
        BOOST_CHECK(static_cast<bool>(avoid_nodebug));
        BOOST_CHECK(step_through_regex.empty());
        handler_called = true;
        h(async::result<>{});
    });

    grp.exec({"step-out", "-A", "true"}, [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(handler_called);
}


/// Tests step out command with -s option
BOOST_AUTO_TEST_CASE(test_step_out_step_through_regex) {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};

    bool handler_called = false;
    MOCK_ADD_CALL(thread_proc, step_out, [&handler_called](auto avoid_nodebug,
                                                           auto step_through_regex,
                                                           const auto & h) {
        BOOST_CHECK(boost::indeterminate(avoid_nodebug));
        BOOST_CHECK(step_through_regex == "foo");
        handler_called = true;
        h(async::result<>{});
    });

    grp.exec({"step-out", "-s", "foo"}, [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(handler_called);
}



/// Tests step inst into command
BOOST_AUTO_TEST_CASE(test_step_inst_into) {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};

    bool handler_called = false;
    MOCK_ADD_CALL(thread_proc, inst_step_into, [&handler_called](const auto & h) {
        handler_called = true;
        h(async::result<>{});
    });

    grp.exec({"step-inst"}, [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(handler_called);
}


/// Tests step inst over command
BOOST_AUTO_TEST_CASE(test_step_inst_over) {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};

    bool handler_called = false;
    MOCK_ADD_CALL(thread_proc, inst_step_over, [&handler_called](const auto & h) {
        handler_called = true;
        h(async::result<>{});
    });

    grp.exec({"step-inst-over"}, [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(handler_called);
}


/// Tests until command
BOOST_AUTO_TEST_CASE(test_until) {
    mock_exec_processor exec_proc;
    mock_thread_processor thread_proc;
    thread_group grp{exec_proc, thread_proc};

    bool handler_called = false;
    MOCK_ADD_CALL(thread_proc, step_until, [&handler_called](const auto line, const auto & h) {
        BOOST_CHECK(line == 12);
        handler_called = true;
        h(async::result<>{});
    });

    grp.exec({"until", "12"}, [](const auto & res) {
        BOOST_CHECK(res == "stepping to line 12");
    });

    BOOST_CHECK(handler_called);
}


BOOST_AUTO_TEST_SUITE_END()


}
