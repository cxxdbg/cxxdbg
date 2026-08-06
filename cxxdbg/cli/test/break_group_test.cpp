// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file break_group_test.cpp
/// Contains unit tests for the break_group class.

#include "mock_code_break_processor.hpp"
#include "mock_exec_processor.hpp"
#include "cxxdbg/cli/break_group.hpp"
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <stack>


namespace cxxdbg::dbg::cli::test {


struct break_group_test_fixture {
    mock_exec_processor exec_proc;
    mock_code_break_processor break_proc;
    break_group break_grp{exec_proc, break_proc};
};


BOOST_FIXTURE_TEST_SUITE(break_group_test, break_group_test_fixture)


/// Tests breakpoint set command via -b option
BOOST_AUTO_TEST_CASE(set_func_test) {
    MOCK_ADD_CALL(break_proc, add_function_breakpoint, [](auto && name, auto && props, auto && h) {
        BOOST_CHECK(name == "my_func");
        BOOST_CHECK(!props.enable_hit_count);
        BOOST_CHECK(props.hit_count == 0);
        BOOST_CHECK(props.cond == "");
        h(async::ok_result(12));
    });

    bool hcalled = false;
    break_grp.exec({"set", "-b", "my_func"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "added breakpoint 12");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests breakpoint set command via -f -l option
BOOST_AUTO_TEST_CASE(set_srcpos_test) {
    MOCK_ADD_CALL(break_proc, add_srcpos_breakpoint, [](auto && file, auto && line, auto && props, auto && h) {
        BOOST_CHECK(file == "foo.cpp");
        BOOST_CHECK(line == 111);
        BOOST_CHECK(!props.enable_hit_count);
        BOOST_CHECK(props.hit_count == 0);
        BOOST_CHECK(props.cond == "");
        h(async::ok_result(12));
    });

    bool hcalled = false;
    break_grp.exec({"set", "-f", "foo.cpp", "-l", "111"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "added breakpoint 12");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests breakpoint set command with missing -l
BOOST_AUTO_TEST_CASE(set_srcpos_no_line_test) {
    bool hcalled = false;
    break_grp.exec({"set", "-f", "foo.cpp"}, [&hcalled](auto && res) {
        hcalled = true;
        BOOST_CHECK(res == "line number is not specified");
    });

    BOOST_CHECK(hcalled);
}


/// Tests breakpoint set command with missing -f
BOOST_AUTO_TEST_CASE(set_srcpos_no_file_test) {
    bool hcalled = false;
    break_grp.exec({"set", "-l", "222"}, [&hcalled](auto && res) {
        hcalled = true;
        BOOST_CHECK(res == "file name is not specified");
    });

    BOOST_CHECK(hcalled);
}


/// Tests breakpoint set command via -a option
BOOST_AUTO_TEST_CASE(set_addr_test) {
    MOCK_ADD_CALL(break_proc, add_address_breakpoint, [](auto && addr, auto && props, auto && h) {
        BOOST_CHECK(addr == 123456);
        BOOST_CHECK(!props.enable_hit_count);
        BOOST_CHECK(props.hit_count == 0);
        BOOST_CHECK(props.cond == "");
        h(async::ok_result(12));
    });

    bool hcalled = false;
    break_grp.exec({"set", "-a", "123456"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "added breakpoint 12");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests breakpoint set command via -a option with hex value
BOOST_AUTO_TEST_CASE(set_addr_hex_test) {
    MOCK_ADD_CALL(break_proc, add_address_breakpoint, [](auto && addr, auto && props, auto && h) {
        BOOST_CHECK(addr == 0xAABBCC12);
        BOOST_CHECK(!props.enable_hit_count);
        BOOST_CHECK(props.hit_count == 0);
        BOOST_CHECK(props.cond == "");
        h(async::ok_result(12));
    });

    bool hcalled = false;
    break_grp.exec({"set", "-a", "0xAABBCC12"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "added breakpoint 12");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests breakpoint set command with -f/-l and -b specified
BOOST_AUTO_TEST_CASE(set_srcpos_func_test) {
    std::vector<std::string> opts{"set", "-f", "foo.cpp", "-l", "222", "-b", "func"};
    bool hcalled = false;
    break_grp.exec(opts, [&hcalled](auto && res) {
        hcalled = true;
        BOOST_CHECK(res == "both file and function specified");
    });

    BOOST_CHECK(hcalled);
}


/// Tests breakpoint set command with -f/-l and -a specified
BOOST_AUTO_TEST_CASE(set_srcpos_addr_test) {
    std::vector<std::string> opts{"set", "-f", "foo.cpp", "-l", "222", "-a", "1111"};
    bool hcalled = false;
    break_grp.exec(opts, [&hcalled](auto && res) {
        hcalled = true;
        BOOST_CHECK(res == "both file and address specified");
    });

    BOOST_CHECK(hcalled);
}


/// Tests breakpoint set command with -b and -a specified
BOOST_AUTO_TEST_CASE(set_func_addr_test) {
    std::vector<std::string> opts{"set", "-b", "func", "-a", "1111"};
    bool hcalled = false;
    break_grp.exec(opts, [&hcalled](auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "both function and address specified");
    });

    BOOST_CHECK(hcalled);
}


/// Tests breakpoint set command with hit count property
BOOST_AUTO_TEST_CASE(set_func_hit_count_test) {
    MOCK_ADD_CALL(break_proc, add_function_breakpoint, [](auto && name, auto && props, auto && h) {
        BOOST_CHECK(name == "my_func");
        BOOST_CHECK(props.enable_hit_count);
        BOOST_CHECK(props.hit_count == 22);
        BOOST_CHECK(props.cond == "");
        h(async::ok_result(12));
    });

    bool hcalled = false;
    break_grp.exec({"set", "-b", "my_func", "-h", "22"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "added breakpoint 12");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests breakpoint set command with condition
BOOST_AUTO_TEST_CASE(set_func_cond_test) {
    MOCK_ADD_CALL(break_proc, add_function_breakpoint, [](auto && name, auto && props, auto && h) {
        BOOST_CHECK(name == "my_func");
        BOOST_CHECK(!props.enable_hit_count);
        BOOST_CHECK(props.hit_count == 0);
        BOOST_CHECK(props.cond == "mycond");
        h(async::ok_result(12));
    });

    bool hcalled = false;
    break_grp.exec({"set", "-b", "my_func", "-c", "mycond"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "added breakpoint 12");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests breakpoint set command with positional function name
BOOST_AUTO_TEST_CASE(set_func_pos_test) {
    MOCK_ADD_CALL(break_proc, add_function_breakpoint, [](auto && name, auto && props, auto && h) {
        BOOST_CHECK(name == "my_func");
        BOOST_CHECK(!props.enable_hit_count);
        BOOST_CHECK(props.hit_count == 0);
        BOOST_CHECK(props.cond == "");
        h(async::ok_result(12));
    });

    bool hcalled = false;
    break_grp.exec({"set", "my_func"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "added breakpoint 12");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests breakpoint set command with positional function name with colons
BOOST_AUTO_TEST_CASE(set_func_pos_test_colons) {
    MOCK_ADD_CALL(break_proc, add_function_breakpoint, [](auto && name, auto && props, auto && h) {
        BOOST_CHECK(name == "my_rec::my_func");
        BOOST_CHECK(!props.enable_hit_count);
        BOOST_CHECK(props.hit_count == 0);
        BOOST_CHECK(props.cond == "");
        h(async::ok_result(12));
    });

    bool hcalled = false;
    break_grp.exec({"set", "my_rec::my_func"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "added breakpoint 12");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests breakpoint set command with positional address
BOOST_AUTO_TEST_CASE(set_addr_pos_test) {
    MOCK_ADD_CALL(break_proc, add_address_breakpoint, [](auto && addr, auto && props, auto && h) {
        BOOST_CHECK(addr = 2222);
        BOOST_CHECK(!props.enable_hit_count);
        BOOST_CHECK(props.hit_count == 0);
        BOOST_CHECK(props.cond == "");
        h(async::ok_result(12));
    });

    bool hcalled = false;
    break_grp.exec({"set", "*2222"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "added breakpoint 12");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests breakpoint set command with positional hex address
BOOST_AUTO_TEST_CASE(set_addr_hex_pos_test) {
    MOCK_ADD_CALL(break_proc, add_address_breakpoint, [](auto && addr, auto && props, auto && h) {
        BOOST_CHECK(addr = 0x2300);
        BOOST_CHECK(!props.enable_hit_count);
        BOOST_CHECK(props.hit_count == 0);
        BOOST_CHECK(props.cond == "");
        h(async::ok_result(12));
    });

    bool hcalled = false;
    break_grp.exec({"set", "*0x002300"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "added breakpoint 12");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests breakpoint set command with positional source position
BOOST_AUTO_TEST_CASE(set_srcpos_pos_test) {
    MOCK_ADD_CALL(break_proc, add_srcpos_breakpoint, [](auto && file, auto && line, auto && props, auto && h) {
        BOOST_CHECK(file == "aaaa/bbb");
        BOOST_CHECK(line == 234);
        BOOST_CHECK(!props.enable_hit_count);
        BOOST_CHECK(props.hit_count == 0);
        BOOST_CHECK(props.cond == "");
        h(async::ok_result(12));
    });

    bool hcalled = false;
    break_grp.exec({"set", "aaaa/bbb:234"}, [&](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "added breakpoint 12");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests breakpoint set command with bad positional position
BOOST_AUTO_TEST_CASE(set_bad_pos_test) {
    bool hcalled = false;
    break_grp.exec({"set", "assss:sdasd"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "line number in location is invalid: 'sdasd'");
    });

    BOOST_CHECK(hcalled);
}


/// Tests set breakpoint when exception thrown
BOOST_AUTO_TEST_CASE(set_exception_thrown) {
    MOCK_ADD_CALL(break_proc, add_exception_thrown_breakpoint, [](auto && props, auto && h) {
        BOOST_CHECK(!props.enable_hit_count);
        BOOST_CHECK(props.hit_count == 0);
        BOOST_CHECK(props.cond == "");
        h(async::ok_result(12));
    });

    bool hcalled = false;
    break_grp.exec({"set", "--exception-thrown"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "added breakpoint 12");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests set breakpoint when exception caught
BOOST_AUTO_TEST_CASE(set_exception_caught) {
    MOCK_ADD_CALL(break_proc, add_exception_caught_breakpoint, [](auto && props, auto && h) {
        BOOST_CHECK(!props.enable_hit_count);
        BOOST_CHECK(props.hit_count == 0);
        BOOST_CHECK(props.cond == "");
        h(async::ok_result(34));
    });

    bool hcalled = false;
    break_grp.exec({"set", "--exception-caught"}, [&hcalled](const auto & res) {
        hcalled = true;
        BOOST_CHECK(res == "added breakpoint 34");
    });

    BOOST_CHECK(hcalled);
    BOOST_CHECK(break_proc.verify());
}


/// Tests breakpoint set command with acceptable combination of --exception-thrown and --exception-caught
BOOST_AUTO_TEST_CASE(set_acceptable_combination) {
    MOCK_ADD_CALL(break_proc, add_exception_thrown_breakpoint, [](auto && props, auto && h) {
        BOOST_CHECK(!props.enable_hit_count);
        BOOST_CHECK(props.hit_count == 0);
        BOOST_CHECK(props.cond == "");
        h(async::ok_result(12));
    });

    MOCK_ADD_CALL(break_proc, add_exception_caught_breakpoint, [](auto && props, auto && h) {
        BOOST_CHECK(!props.enable_hit_count);
        BOOST_CHECK(props.hit_count == 0);
        BOOST_CHECK(props.cond == "");
        h(async::ok_result(34));
    });

    std::stack<std::string> messages;
    messages.push("added breakpoint 34");
    messages.push("added breakpoint 12");

    break_grp.exec({"set", "--exception-thrown", "--exception-caught"}, [&messages](const auto & res) {
        if (messages.size() > 0) {
            BOOST_CHECK(res == messages.top());
            messages.pop();
        } else {
            BOOST_FAIL("no messages left");
        }
    });

    BOOST_CHECK(messages.size() == 0); // all messages consumed

    BOOST_CHECK(break_proc.verify());
}


/// Tests breakpoint set command with wrong combination of --exception-thrown and --exception-caught with other options
BOOST_AUTO_TEST_CASE(set_wrong_combination) {
    {
        // file option
        bool handler_called = false;
        break_grp.exec({"set", "--exception-thrown", "--file", "asd.cpp"}, [&handler_called](const auto & res) {
            handler_called = true;
            BOOST_CHECK(res == "cannot combine exception-thrown or exception-caught options with location, function, address, file or line options");
        });

        BOOST_CHECK(handler_called);
    }

    {
        // locaiton option
        bool handler_called = false;
        break_grp.exec({"set", "--exception-thrown", "--location", "asdfgh"}, [&handler_called](const auto & res) {
            handler_called = true;
            BOOST_CHECK(res == "cannot combine exception-thrown or exception-caught options with location, function, address, file or line options");
        });

        BOOST_CHECK(handler_called);
    }

    {
        // address option
        bool handler_called = false;
        break_grp.exec({"set", "--exception-thrown", "--address", "0x002300"}, [&handler_called](const auto & res) {
            handler_called = true;
            BOOST_CHECK(res == "cannot combine exception-thrown or exception-caught options with location, function, address, file or line options");
        });

        BOOST_CHECK(handler_called);
    }

    {
        // function option
        bool handler_called = false;
        break_grp.exec({"set", "--exception-thrown", "--function", "foo"}, [&handler_called](const auto & res) {
            handler_called = true;
            BOOST_CHECK(res == "cannot combine exception-thrown or exception-caught options with location, function, address, file or line options");
        });

        BOOST_CHECK(handler_called);
    }

    {
        // function option
        bool handler_called = false;
        break_grp.exec({"set", "--exception-thrown", "--file", "foo.cpp"}, [&handler_called](const auto & res) {
            handler_called = true;
            BOOST_CHECK(res == "cannot combine exception-thrown or exception-caught options with location, function, address, file or line options");
        });

        BOOST_CHECK(handler_called);
    }

    {
        // function option
        bool handler_called = false;
        break_grp.exec({"set", "--exception-thrown", "--line", "123"}, [&handler_called](const auto & res) {
            handler_called = true;
            BOOST_CHECK(res == "cannot combine exception-thrown or exception-caught options with location, function, address, file or line options");
        });

        BOOST_CHECK(handler_called);
    }
}


BOOST_AUTO_TEST_SUITE_END()


}
