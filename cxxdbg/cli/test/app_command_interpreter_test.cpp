// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file app_command_interpreter_test.cpp
/// Contains unit tests for the app_command_interpreter class.

#include "mock_code_break_processor.hpp"
#include "mock_exec_processor.hpp"
#include "mock_frame_processor.hpp"
#include "mock_platform_processor.hpp"
#include "mock_process_processor.hpp"
#include "mock_target_processor.hpp"
#include "mock_thread_processor.hpp"
#include "mock_watch_processor.hpp"
#include "cxxdbg/cli/app_command_interpreter.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::cli::test {


/// Helper class for initializing app interpreter with mock processor
struct app_interp_fixture {
    mock_code_break_processor break_proc;
    mock_exec_processor exec_proc;
    mock_frame_processor frame_proc;
    mock_process_processor proc_proc;
    mock_target_processor targ_proc;
    mock_thread_processor thread_proc;
    mock_watch_processor watch_proc;
    mock_platform_processor platform_proc;

    app_command_interpreter interp;

    app_interp_fixture():
    interp{break_proc, exec_proc, frame_proc, proc_proc, targ_proc, thread_proc, watch_proc, platform_proc} {
    }
};


BOOST_FIXTURE_TEST_SUITE(app_command_interpreter_test, app_interp_fixture)


#define TEST_APP_EXEC_COMMAND_REGISTERED_STR(name, namestr) \
    BOOST_AUTO_TEST_CASE(name##_test) { \
        MOCK_ADD_CALL(exec_proc, exec_cmd, [](auto && cmd, auto && handl) { \
            BOOST_CHECK(cmd == namestr " aaa"); \
        }); \
        interp.exec(namestr " aaa", [](const auto & res) {}); \
        BOOST_CHECK(exec_proc.verify()); \
    }

#define TEST_APP_EXEC_COMMAND_REGISTERED(name) \
    TEST_APP_EXEC_COMMAND_REGISTERED_STR(name, #name)

#define TEST_ALIAS_EXEC_COMMAND_REGISTERED_STR(name, namestr, alias) \
    BOOST_AUTO_TEST_CASE(name##_test_alias) { \
        MOCK_ADD_CALL(exec_proc, exec_cmd, [](auto && cmd, auto && handl) { \
            BOOST_CHECK(cmd == alias " aaa"); \
        }); \
        interp.exec(namestr " aaa", [](const auto & res) {}); \
        BOOST_CHECK(exec_proc.verify()); \
    }

#define TEST_ALIAS_EXEC_COMMAND_REGISTERED(name, alias) \
    TEST_ALIAS_EXEC_COMMAND_REGISTERED_STR(name, #name, alias)



TEST_APP_EXEC_COMMAND_REGISTERED_STR(target, "target list")
TEST_APP_EXEC_COMMAND_REGISTERED_STR(process, "process status")


TEST_APP_EXEC_COMMAND_REGISTERED(disassemble)
TEST_APP_EXEC_COMMAND_REGISTERED(expression)
TEST_APP_EXEC_COMMAND_REGISTERED(memory)
TEST_APP_EXEC_COMMAND_REGISTERED(plugin)
TEST_APP_EXEC_COMMAND_REGISTERED(register)
TEST_APP_EXEC_COMMAND_REGISTERED(settings)
TEST_APP_EXEC_COMMAND_REGISTERED(source)
TEST_APP_EXEC_COMMAND_REGISTERED(type)
TEST_APP_EXEC_COMMAND_REGISTERED(list)
TEST_APP_EXEC_COMMAND_REGISTERED(log)



TEST_ALIAS_EXEC_COMMAND_REGISTERED_STR(add_dsym, "add-dsym",   "target symbols add")

TEST_ALIAS_EXEC_COMMAND_REGISTERED(bt,          "thread backtrace")
TEST_ALIAS_EXEC_COMMAND_REGISTERED(call,        "expression --")
TEST_ALIAS_EXEC_COMMAND_REGISTERED(di,          "disassemble")
TEST_ALIAS_EXEC_COMMAND_REGISTERED(dis,         "disassemble")
TEST_ALIAS_EXEC_COMMAND_REGISTERED(expr,        "expression")
TEST_ALIAS_EXEC_COMMAND_REGISTERED(e,           "expression")
TEST_ALIAS_EXEC_COMMAND_REGISTERED(image,       "target modules")
TEST_ALIAS_EXEC_COMMAND_REGISTERED(p,           "expression --")
TEST_ALIAS_EXEC_COMMAND_REGISTERED(po,          "expression -O --")
TEST_ALIAS_EXEC_COMMAND_REGISTERED(print,       "expression --")
TEST_ALIAS_EXEC_COMMAND_REGISTERED(x,           "memory read")


/// Tests attach alias
BOOST_AUTO_TEST_CASE(test_attach) {
    MOCK_ADD_CALL(proc_proc, attach_pid, [](const auto & pid, const auto & h) {
        BOOST_CHECK(pid == 1232);
        h(async::ok_result(pid));
    });

    interp.exec("attach 1232", [](const auto & res) {
        BOOST_CHECK(res == "attached to process 1232");
    });
}


/// Tests b alias
BOOST_AUTO_TEST_CASE(test_b_alias) {
    MOCK_ADD_CALL(break_proc, add_srcpos_breakpoint, [](auto && file, auto && line, auto && props, auto && h) {
        BOOST_CHECK(file == "test.cpp");
        BOOST_CHECK(line == 10);
        h(async::ok_result(88));
    });

    interp.exec("b test.cpp:10", [](const auto & res) {
        BOOST_CHECK(res == "added breakpoint 88");
    });

    BOOST_CHECK(break_proc.verify());
}


/// Tests c alias
BOOST_AUTO_TEST_CASE(test_c_alias) {
    MOCK_ADD_CALL(proc_proc, resume, [](const auto & h) {
        h(async::ok_result(111));
    });

    interp.exec("c", [](const auto & res) {
        BOOST_CHECK(res == "continuing process 111");
    });
}


/// Tests continue alias
BOOST_AUTO_TEST_CASE(test_continue_alias) {
    MOCK_ADD_CALL(proc_proc, resume, [](const auto & h) {
        h(async::ok_result(111));
    });

    interp.exec("continue", [](const auto & res) {
        BOOST_CHECK(res == "continuing process 111");
    });
}


/// Tests d alias
BOOST_AUTO_TEST_CASE(test_d_alias) {
    MOCK_ADD_CALL(break_proc, delete_breakpoint, [](auto && id, auto && h) {
        BOOST_CHECK(id.type() == breakpoint_type::code);
        BOOST_CHECK(id.num() == 34);
        h(async::ok_result(88));
    });

    interp.exec("d 34", [](const auto & res) {
        BOOST_CHECK_EQUAL(res, "deleted breakpoint 34");
    });

    BOOST_CHECK(break_proc.verify());
}


/// Tests f alias
BOOST_AUTO_TEST_CASE(test_f_alias) {
    bool h_called = false;
    MOCK_ADD_CALL(frame_proc, select_frame, [&h_called](auto index, const auto & h) {
        h_called = true;
        BOOST_CHECK(index == 232);
        h(async::result<>{});
    });

    interp.exec("f 232", [](const auto & res) {
        BOOST_CHECK(res == "selected frame 232");
    });

    BOOST_CHECK(h_called);
}


/// Tests finish alias
BOOST_AUTO_TEST_CASE(test_finish_alias) {
    bool h_called = false;
    MOCK_ADD_CALL(thread_proc, step_out, [&h_called](auto avoid_nodebug,
                                                     auto step_through_regex,
                                                     const auto & h) {
        BOOST_CHECK(boost::indeterminate(avoid_nodebug));
        BOOST_CHECK(step_through_regex.empty());
        h_called = true;
        h(async::result<>{});
    });

    interp.exec("finish", [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(h_called);
}


/// Tests kill alias
BOOST_AUTO_TEST_CASE(test_kill_alias) {
    MOCK_ADD_CALL(proc_proc, kill, [](const auto & h) {
        h(async::ok_result(1113));
    });

    interp.exec("kill", [](const auto & res) {
        BOOST_CHECK(res == "killed process 1113");
    });
}


/// Tests detach alias
BOOST_AUTO_TEST_CASE(test_detach_alias) {
    MOCK_ADD_CALL(proc_proc, detach, [](const auto & h) {
        h(async::ok_result(1113));
    });

    interp.exec("detach", [](const auto & res) {
        BOOST_CHECK(res == "detached from process 1113");
    });
}


/// Tests n alias
BOOST_AUTO_TEST_CASE(test_n_alias) {
    bool h_called = false;
    MOCK_ADD_CALL(thread_proc, step_over, [&h_called](auto step_through_regexp,
                                                      const auto & h) {
        h_called = true;
        BOOST_CHECK(step_through_regexp.empty());
        h(async::result<>{});
    });

    interp.exec("n", [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(h_called);
}


/// Tests next alias
BOOST_AUTO_TEST_CASE(test_next_alias) {
    bool h_called = false;
    MOCK_ADD_CALL(thread_proc, step_over, [&h_called](auto step_through_regexp,
                                                      const auto & h) {
        h_called = true;
        BOOST_CHECK(step_through_regexp.empty());
        h(async::result<>{});
    });

    interp.exec("next", [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(h_called);
}


/// Tests nexti alias
BOOST_AUTO_TEST_CASE(test_nexti_alias) {
    bool h_called = false;
    MOCK_ADD_CALL(thread_proc, inst_step_over, [&h_called](const auto & h) {
        h_called = true;
        h(async::result<>{});
    });

    interp.exec("nexti", [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(h_called);
}


/// Tests ni alias
BOOST_AUTO_TEST_CASE(test_ni_alias) {
    bool h_called = false;
    MOCK_ADD_CALL(thread_proc, inst_step_over, [&h_called](const auto & h) {
        h_called = true;
        h(async::result<>{});
    });

    interp.exec("ni", [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(h_called);
}


/// Tests s alias
BOOST_AUTO_TEST_CASE(test_s_alias) {
    bool h_called = false;
    MOCK_ADD_CALL(thread_proc, step_into, [&h_called](auto avoid_nodebug,
                                                      auto target_func,
                                                      auto avoid_regex,
                                                      auto step_through_regex,
                                                      const auto & h) {
        BOOST_CHECK(boost::indeterminate(avoid_nodebug));
        BOOST_CHECK(target_func.empty());
        BOOST_CHECK(avoid_regex.empty());
        BOOST_CHECK(step_through_regex.empty());
        h_called = true;
        h(async::result<>{});
    });

    interp.exec("s", [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(h_called);
}


/// Tests si alias
BOOST_AUTO_TEST_CASE(test_si_alias) {
    bool h_called = false;
    MOCK_ADD_CALL(thread_proc, inst_step_into, [&h_called](const auto & h) {
        h_called = true;
        h(async::result<>{});
    });

    interp.exec("si", [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(h_called);
}


/// Tests step alias
BOOST_AUTO_TEST_CASE(test_step_alias) {
    bool h_called = false;
    MOCK_ADD_CALL(thread_proc, step_into, [&h_called](auto avoid_nodebug,
                                                      auto target_func,
                                                      auto avoid_regex,
                                                      auto step_through_regex,
                                                      const auto & h) {
        BOOST_CHECK(boost::indeterminate(avoid_nodebug));
        BOOST_CHECK(target_func.empty());
        BOOST_CHECK(avoid_regex.empty());
        BOOST_CHECK(step_through_regex.empty());
        h_called = true;
        h(async::result<>{});
    });

    interp.exec("step", [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(h_called);
}


/// Tests stepi alias
BOOST_AUTO_TEST_CASE(test_stepi_alias) {
    bool h_called = false;
    MOCK_ADD_CALL(thread_proc, inst_step_into, [&h_called](const auto & h) {
        h_called = true;
        h(async::result<>{});
    });

    interp.exec("stepi", [](const auto & res) {
        BOOST_CHECK(res == "stepping");
    });

    BOOST_CHECK(h_called);
}


/// Tests r alias
BOOST_AUTO_TEST_CASE(test_r_alias) {
    bool handler_called = false;
    MOCK_ADD_CALL(proc_proc, launch, [&handler_called](const auto & ... args) {
        handler_called = true;
    });

    interp.exec("r", [](const auto & res) {
    });

    BOOST_CHECK(handler_called);
}


/// Tests r alias
BOOST_AUTO_TEST_CASE(test_run_alias) {
    bool handler_called = false;
    MOCK_ADD_CALL(proc_proc, launch, [&handler_called](const auto & ... args) {
        handler_called = true;
    });

    interp.exec("run", [](const auto & res) {
    });

    BOOST_CHECK(handler_called);
}


/// Tests t alias
BOOST_AUTO_TEST_CASE(test_t_alias) {
    bool h_called = false;
    MOCK_ADD_CALL(thread_proc, select_thread, [&h_called](auto index, const auto & h) {
        BOOST_CHECK(index == 200);
        h_called = true;
        h(async::result<>{});
    });

    interp.exec("t 200", [](const auto & res) {
        BOOST_CHECK(res == "selected thread 200");
    });

    BOOST_CHECK(h_called);
}


BOOST_AUTO_TEST_SUITE_END()


}
