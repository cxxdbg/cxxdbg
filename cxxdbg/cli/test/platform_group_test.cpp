// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file platform_group_test.cpp
/// Contains unit tests for the platform_group class.

#include "mock_exec_processor.hpp"
#include "mock_platform_processor.hpp"
#include "../platform_group.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::cli::test {


struct platform_group_test_fixture {
    mock_exec_processor exec_proc;
    mock_platform_processor platform_proc;
    platform_group grp{exec_proc, platform_proc};
};


#define TEST_APP_EXEC_COMMAND_REGISTERED(name, namestr) \
    BOOST_AUTO_TEST_CASE(name##_test) { \
        MOCK_ADD_CALL(exec_proc, exec_cmd, [](auto && cmd, auto && handl) { \
            BOOST_CHECK(cmd == "platform " namestr " arg1"); \
        }); \
        grp.exec({namestr, "arg1"}, [](const auto & res) {}); \
        \
        BOOST_CHECK(exec_proc.verify()); \
    }


BOOST_FIXTURE_TEST_SUITE(platform_group_test, platform_group_test_fixture)


TEST_APP_EXEC_COMMAND_REGISTERED(file, "file")
TEST_APP_EXEC_COMMAND_REGISTERED(get_file, "get-file")
TEST_APP_EXEC_COMMAND_REGISTERED(get_size, "get-size")
TEST_APP_EXEC_COMMAND_REGISTERED(list, "list")
TEST_APP_EXEC_COMMAND_REGISTERED(mkdir, "mkdir")
TEST_APP_EXEC_COMMAND_REGISTERED(process, "process")
TEST_APP_EXEC_COMMAND_REGISTERED(put_file, "put-file")
TEST_APP_EXEC_COMMAND_REGISTERED(settings, "settings")
TEST_APP_EXEC_COMMAND_REGISTERED(shell, "shell")
TEST_APP_EXEC_COMMAND_REGISTERED(status, "status")
TEST_APP_EXEC_COMMAND_REGISTERED(target_install, "target-install")


/// Tests successfull selecting platform
BOOST_AUTO_TEST_CASE(test_select) {
    MOCK_ADD_CALL(platform_proc, select_platform, [](std::string_view name) {
        BOOST_CHECK(name == "my-platform");
        return true;
    });

    bool handler_called = false;
    grp.exec({"select", "my-platform"}, [&handler_called](auto && res) {
        BOOST_CHECK_EQUAL(res, "selected platform: my-platform");
        handler_called = true;
    });

    BOOST_CHECK(handler_called);
    BOOST_CHECK(platform_proc.verify());
}


/// Tests error selecting platform
BOOST_AUTO_TEST_CASE(test_select_error) {
    MOCK_ADD_CALL(platform_proc, select_platform, [](std::string_view name) {
        BOOST_CHECK(name == "my-platform");
        return false;
    });

    bool handler_called = false;
    grp.exec({"select", "my-platform"}, [&handler_called](auto && res) {
        BOOST_CHECK_EQUAL(res, "error: platform not found: my-platform");
        handler_called = true;
    });

    BOOST_CHECK(handler_called);
    BOOST_CHECK(platform_proc.verify());
}


/// Tests successfull connecting to platform
BOOST_AUTO_TEST_CASE(test_connect) {
    MOCK_ADD_CALL(platform_proc, connect_to_platform, [](const std::string_view url,
                                                         const std::string & local_cache_dir,
                                                         bool enable_rsync,
                                                         const std::string & rsync_opts,
                                                         const std::string & rsync_prefix,
                                                         bool ignore_remote_host_name,
                                                         const async::result_handler<std::string> & handler) {
        BOOST_CHECK_EQUAL(url, "xurl");
        BOOST_CHECK_EQUAL(local_cache_dir, "my-local-path");
        BOOST_CHECK(!enable_rsync);
        BOOST_CHECK(rsync_opts.empty());
        BOOST_CHECK(rsync_prefix.empty());
        BOOST_CHECK(!ignore_remote_host_name);
        async::result<std::string> res{""};
        res.set_value("my-platform");
        handler(res);
    });

    bool handler_called = false;
    grp.exec({"connect", "-c", "my-local-path", "xurl"}, [&handler_called](auto && res) {
        BOOST_CHECK_EQUAL(res, "connected to platform my-platform with URL xurl");
        handler_called = true;
    });

    BOOST_CHECK(handler_called);
    BOOST_CHECK(platform_proc.verify());
}


/// Tests error connecting to platform
BOOST_AUTO_TEST_CASE(test_connect_error) {
    MOCK_ADD_CALL(platform_proc, connect_to_platform, [](const std::string_view url,
                                                         const std::string & local_cache_dir,
                                                         bool enable_rsync,
                                                         const std::string & rsync_opts,
                                                         const std::string & rsync_prefix,
                                                         bool ignore_remote_host_name,
                                                         const async::result_handler<std::string> & handler) {
        BOOST_CHECK_EQUAL(url, "xurl");
        BOOST_CHECK_EQUAL(local_cache_dir, "my-local-path");
        BOOST_CHECK(!enable_rsync);
        BOOST_CHECK(rsync_opts.empty());
        BOOST_CHECK(rsync_prefix.empty());
        BOOST_CHECK(!ignore_remote_host_name);
        handler(async::result<std::string>{"my error"});
    });

    bool handler_called = false;
    grp.exec({"connect", "-c", "my-local-path", "xurl"}, [&handler_called](auto && res) {
        BOOST_CHECK_EQUAL(res, "can't connect to platform: my error");
        handler_called = true;
    });

    BOOST_CHECK(handler_called);
    BOOST_CHECK(platform_proc.verify());
}


/// Tests successfull connecting to platform with rsync options
BOOST_AUTO_TEST_CASE(test_connect_rsync) {
    MOCK_ADD_CALL(platform_proc, connect_to_platform, [](const std::string_view url,
                                                         const std::string & local_cache_dir,
                                                         bool enable_rsync,
                                                         const std::string & rsync_opts,
                                                         const std::string & rsync_prefix,
                                                         bool ignore_remote_host_name,
                                                         const async::result_handler<std::string> & handler) {
        BOOST_CHECK_EQUAL(url, "xurl");
        BOOST_CHECK_EQUAL(local_cache_dir, "my-local-path");
        BOOST_CHECK(enable_rsync);
        BOOST_CHECK_EQUAL(rsync_opts, "my rsync opts");
        BOOST_CHECK_EQUAL(rsync_prefix, "my rsync prefix");
        BOOST_CHECK(ignore_remote_host_name);
        async::result<std::string> res{""};
        res.set_value("my-platform");
        handler(res);
    });

    bool handler_called = false;
    grp.exec({"connect", "-c", "my-local-path", "xurl", "-P" "my rsync prefix", "-R", "my rsync opts", "-i", "-r"},
    [&handler_called](auto && res) {
        BOOST_CHECK_EQUAL(res, "connected to platform my-platform with URL xurl");
        handler_called = true;
    });

    BOOST_CHECK(handler_called);
    BOOST_CHECK(platform_proc.verify());
}


/// Tests disconnecting from platform
BOOST_AUTO_TEST_CASE(test_disconnect) {
    MOCK_ADD_CALL(platform_proc, disconnect_from_platform, [] {});

    bool handler_called = false;
    grp.exec({"disconnect"}, [&handler_called](auto && res) {
        BOOST_CHECK_EQUAL(res, "disconnected");
        handler_called = true;
    });

    BOOST_CHECK(handler_called);
    BOOST_CHECK(platform_proc.verify());
}


BOOST_AUTO_TEST_SUITE_END()



}
