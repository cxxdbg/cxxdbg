// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file process_group_test.cpp
/// Contains unit tests for the process_group class.

#include "mock_exec_processor.hpp"
#include "mock_process_processor.hpp"
#include "cxxdbg/cli/process_group.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::cli::test {


struct process_group_test_fixture {
    mock_exec_processor exec_proc;
    mock_process_processor proc_proc;
    process_group grp{exec_proc, proc_proc};
};


#define TEST_APP_EXEC_COMMAND_REGISTERED(name, namestr) \
    BOOST_AUTO_TEST_CASE(name##_test) { \
        MOCK_ADD_CALL(exec_proc, exec_cmd, [](auto && cmd, auto && handl) { \
            BOOST_CHECK(cmd == "process " namestr " arg1"); \
        }); \
        grp.exec({namestr, "arg1"}, [](const auto & res) {}); \
        \
        BOOST_CHECK(exec_proc.verify()); \
    }


BOOST_FIXTURE_TEST_SUITE(process_group_test, process_group_test_fixture)


TEST_APP_EXEC_COMMAND_REGISTERED(handle, "handle")
TEST_APP_EXEC_COMMAND_REGISTERED(load, "load")
TEST_APP_EXEC_COMMAND_REGISTERED(plugin, "plugin")
TEST_APP_EXEC_COMMAND_REGISTERED(save_core, "save-core")
TEST_APP_EXEC_COMMAND_REGISTERED(signal, "signal")
TEST_APP_EXEC_COMMAND_REGISTERED(status, "status")
TEST_APP_EXEC_COMMAND_REGISTERED(unload, "unload")


static const auto nullfunc = [](const auto & ... args) {};


/// Tests success attaching to process
BOOST_AUTO_TEST_CASE(attach_success) {
    MOCK_ADD_CALL(proc_proc, attach, [](const auto & handl) {
        handl(async::ok_result(15));
    });

    bool hcalled = false;
    grp.exec({"attach"}, [&hcalled](const auto & res) {
        BOOST_CHECK(res == "attached to process 15");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}


/// Tests success attaching to process with error
BOOST_AUTO_TEST_CASE(attach_error) {
    MOCK_ADD_CALL(proc_proc, attach, [](const auto & handl) {
        handl(async::result<unsigned long>{"error"});
    });

    bool hcalled = false;
    grp.exec({"attach"}, [&hcalled](const auto & res) {
        BOOST_CHECK(res == "error");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}


/// Tests success attaching to process with pid
BOOST_AUTO_TEST_CASE(attach_pid_success) {
    MOCK_ADD_CALL(proc_proc, attach_pid, [](unsigned long pid, const auto & handl) {
        BOOST_CHECK(pid == 35);
        handl(async::ok_result(pid));
    });

    bool hcalled = false;
    grp.exec({"attach", "--pid=35"}, [&hcalled](const auto & res) {
        BOOST_CHECK(res == "attached to process 35");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}


/// Tests error attaching to process with pid
BOOST_AUTO_TEST_CASE(attach_pid_error) {
    MOCK_ADD_CALL(proc_proc, attach_pid, [](unsigned long pid, const auto & handl) {
        BOOST_CHECK(pid == 35);
        async::result<unsigned long> res("error");
        handl(res);
    });

    bool hcalled = false;
    grp.exec({"attach", "--pid=35"}, [&hcalled](const auto & res) {
        BOOST_CHECK(res == "error");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}


/// Tests successfull attaching to process with name
BOOST_AUTO_TEST_CASE(attach_name_success) {
    MOCK_ADD_CALL(proc_proc, attach_name, [](const std::string & nm, const auto & handl) {
        BOOST_CHECK(nm == "my_proc");
        handl(async::ok_result(1200));
    });

    bool hcalled = false;
    grp.exec({"attach", "--name=my_proc"}, [&hcalled](const auto & res) {
        BOOST_CHECK(res == "attached to process 1200");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}


/// Tests attaching to process by name in target argument
BOOST_AUTO_TEST_CASE(attach_target_name) {
    MOCK_ADD_CALL(proc_proc, attach_name, [](const std::string & nm, const auto & handl) {
        BOOST_CHECK(nm == "my_proc");
        handl(async::ok_result(1200));
    });

    bool hcalled = false;
    grp.exec({"attach", "my_proc"}, [&hcalled](const auto & res) {
        BOOST_CHECK(res == "attached to process 1200");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}


/// Tests attaching to process by PID in target argument
BOOST_AUTO_TEST_CASE(attach_target_pid) {
    MOCK_ADD_CALL(proc_proc, attach_pid, [](unsigned long pid, const auto & handl) {
        BOOST_CHECK(pid == 1200);
        handl(async::ok_result(1200));
    });

    bool hcalled = false;
    grp.exec({"attach", "1200"}, [&hcalled](const auto & res) {
        BOOST_CHECK(res == "attached to process 1200");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}


/// Tests resuming process
BOOST_AUTO_TEST_CASE(resume_success) {
    MOCK_ADD_CALL(proc_proc, resume, [](const auto & handl) {
        handl(async::ok_result(20));
    });

    bool hcalled = false;
    grp.exec({"continue"}, [&hcalled](const auto & res) {
        BOOST_CHECK(res == "continuing process 20");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}


/// Tests resuming process with error
BOOST_AUTO_TEST_CASE(resume_error) {
    MOCK_ADD_CALL(proc_proc, resume, [](const auto & handl) {
        async::result<unsigned long> res("aaa");
        handl(res);
    });

    bool hcalled = false;
    grp.exec({"continue"}, [&hcalled](const auto & res) {
        BOOST_CHECK(res == "aaa");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}


/// Tests detaching from process
BOOST_AUTO_TEST_CASE(detach_success) {
    MOCK_ADD_CALL(proc_proc, detach, [](const auto & handl) {
        handl(async::ok_result(20));
    });

    bool hcalled = false;
    grp.exec({"detach"}, [&hcalled](const auto & res) {
        BOOST_CHECK(res == "detached from process 20");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}


/// Tests detaching from process with error
BOOST_AUTO_TEST_CASE(detach_error) {
    MOCK_ADD_CALL(proc_proc, detach, [](const auto & handl) {
        async::result<unsigned long> res("aaa");
        handl(res);
    });

    bool hcalled = false;
    grp.exec({"detach"}, [&hcalled](const auto & res) {
        BOOST_CHECK(res == "aaa");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}


/// Tests interrupting process
BOOST_AUTO_TEST_CASE(interrupt_success) {
    MOCK_ADD_CALL(proc_proc, interrupt, [](const auto & handl) {
        handl(async::ok_result(20));
    });

    bool hcalled = false;
    grp.exec({"interrupt"}, [&hcalled](const auto & res) {
        BOOST_CHECK(res == "interrupted process 20");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}


/// Tests interrupting process with error
BOOST_AUTO_TEST_CASE(interrupt_error) {
    MOCK_ADD_CALL(proc_proc, interrupt, [](const auto & handl) {
        async::result<unsigned long> res("aaa");
        handl(res);
    });

    bool hcalled = false;
    grp.exec({"interrupt"}, [&hcalled](const auto & res) {
        BOOST_CHECK(res == "aaa");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}


/// Tests killing process
BOOST_AUTO_TEST_CASE(kill_success) {
    MOCK_ADD_CALL(proc_proc, kill, [](const auto & handl) {
        handl(async::ok_result(20));
    });

    bool hcalled = false;
    grp.exec({"kill"}, [&hcalled](const auto & res) {
        BOOST_CHECK(res == "killed process 20");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}


/// Tests killing process with error
BOOST_AUTO_TEST_CASE(kill_error) {
    MOCK_ADD_CALL(proc_proc, kill, [](const auto & handl) {
        async::result<unsigned long> res("aaa");
        handl(res);
    });

    bool hcalled = false;
    grp.exec({"kill"}, [&hcalled](const auto & res) {
        BOOST_CHECK(res == "aaa");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}


/// Tests launching process with default options
BOOST_AUTO_TEST_CASE(launch) {
    MOCK_ADD_CALL(proc_proc, launch, [](const auto & work_dir, const auto & cmd_args,
                                        bool save, const auto & handl) {
        BOOST_CHECK(work_dir.empty());
        BOOST_CHECK(cmd_args.empty());
        BOOST_CHECK(!save);

        handl(async::ok_result(22));
    });

    bool hcalled = false;
    grp.exec({"launch"}, [&hcalled](const auto & res) {
        BOOST_CHECK(res == "launched process 22");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}


/// Tests launching process with work dir
BOOST_AUTO_TEST_CASE(launch_opts_work_dir) {
    MOCK_ADD_CALL(proc_proc, launch, [](const auto & work_dir, const auto & cmd_args,
                                        bool save, const auto & handl) {
        BOOST_CHECK(work_dir == "aaaa/bbbb");
        BOOST_CHECK(cmd_args.empty());
        BOOST_CHECK(save);

        handl(async::ok_result(22));
    });

    bool hcalled = false;
    grp.exec({"launch", "--work-dir=aaaa/bbbb", "--save-default"},
             [&hcalled](const auto & res) {
        BOOST_CHECK(res == "launched process 22");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}


/// Tests launching process with work dir and command arguments
BOOST_AUTO_TEST_CASE(launch_opts_work_dir_args) {
    MOCK_ADD_CALL(proc_proc, launch, [](const auto & work_dir, const auto & cmd_args,
                                        bool save, const auto & handl) {
        BOOST_CHECK(work_dir == "aaaa/bbbb");
        BOOST_CHECK(cmd_args.size() == 1);
        BOOST_CHECK(cmd_args[0] == "zzz");

        handl(async::ok_result(22));
    });

    bool hcalled = false;
    grp.exec({"launch", "--work-dir=aaaa/bbbb", "--args", "zzz"},
             [&hcalled](const auto & res) {
        BOOST_CHECK(res == "launched process 22");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}



/// Tests launching process with error
BOOST_AUTO_TEST_CASE(launch_error) {
    MOCK_ADD_CALL(proc_proc, launch, [](const auto & work_dir, const auto & cmd_args,
                                        bool save, const auto & handl) {
        async::result<unsigned long> res("myerr");
        handl(res);
    });

    bool hcalled = false;
    grp.exec({"launch"}, [&hcalled](const auto & res) {
        BOOST_CHECK(res == "myerr");
        hcalled = true;
    });

    BOOST_CHECK(proc_proc.verify());
    BOOST_CHECK(hcalled);
}


BOOST_AUTO_TEST_SUITE_END()


}
