// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file launch_options_test.cpp
/// Contains unit tests for launch_options class.

#include "../launch_options.hpp"
#include <boost/test/unit_test.hpp>


namespace fs = std::filesystem;


namespace cxxdbg::dbg::test {


BOOST_AUTO_TEST_SUITE(launch_options_test)


/// Tests launch options initialization
BOOST_AUTO_TEST_CASE(init) {
    launch_options opts;

    BOOST_CHECK(opts.work_dir().empty());
    BOOST_CHECK(opts.launch_args().empty());
}


/// Tests setting work dir
BOOST_AUTO_TEST_CASE(set_work_dir) {
    launch_options opts;
    opts.set_work_dir(fs::path("aaaa"));

    BOOST_CHECK(opts.work_dir() == fs::path("aaaa"));
}


/// Tests setting launch args
BOOST_AUTO_TEST_CASE(set_launch_args) {
    launch_options opts;
    opts.set_launch_args("aaaaa bbbbb ccc");

    BOOST_REQUIRE(opts.launch_args().size() == 3);
    BOOST_CHECK(opts.launch_args().at(0) == "aaaaa");
    BOOST_CHECK(opts.launch_args().at(1) == "bbbbb");
    BOOST_CHECK(opts.launch_args().at(2) == "ccc");
}


/// Tests setting empty launch args
BOOST_AUTO_TEST_CASE(set_empty_launch_args) {
    launch_options opts;
    opts.set_launch_args("");

    BOOST_CHECK(opts.launch_args().empty());
}


/// Tests setting spaces in launch args
BOOST_AUTO_TEST_CASE(set_spaces_launch_args) {
    launch_options opts;
    opts.set_launch_args("    ");

    BOOST_CHECK(opts.launch_args().empty());
}


/// Tests checking that work dir exists
BOOST_AUTO_TEST_CASE(not_existing_work_dir) {
    launch_options opts;
    opts.set_work_dir("/proc/aaaaaa");

    BOOST_CHECK(opts.check() == launch_options::check_result_work_dir_does_not_exist);
}


/// Tests checking that work dir is a directory
BOOST_AUTO_TEST_CASE(not_dir_work_dir) {
    launch_options opts;
#ifdef _WIN32
    opts.set_work_dir("C:/pagefile.sys");
#else
    opts.set_work_dir("/etc/hosts");
#endif

    BOOST_CHECK(opts.check() == launch_options::check_result_work_dir_is_not_a_dir);
}


/// Tests good options
BOOST_AUTO_TEST_CASE(good_opts) {
    launch_options opts;
    opts.set_work_dir("/");

    BOOST_CHECK(opts.check() == launch_options::check_result_ok);
}


/// Tests making launch args string
BOOST_AUTO_TEST_CASE(make_args_string) {
    launch_options opts;
    opts.launch_args().push_back("aa");
    opts.launch_args().push_back("bbbbb");

    BOOST_CHECK(opts.launch_args_string() == "aa bbbbb");
}


BOOST_AUTO_TEST_SUITE_END()


}
