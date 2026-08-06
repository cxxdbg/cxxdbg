// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file command_line_parser_test.cpp
/// Contains unit tests for command line parser for cxxdbgutil library.

#include "cxxdbg/util/command_line_parser.hpp"
#include <boost/test/unit_test.hpp>
#include <sstream>


namespace cxxdbg { namespace util { namespace test {


BOOST_AUTO_TEST_SUITE(command_line_parser_test)


/// Simple test for --args parsing
BOOST_AUTO_TEST_CASE(parse_args_simple) {
    char * argv[] = {
        (char*)"command",
        (char*)"-arg1",
        (char*)"--arg2",
        (char*)"--args",
        (char*)"arg3",
        (char*)"--arg4"
    };

    std::vector<std::string> regular_opts;
    std::vector<std::string> args_opts;

    bool res = parse_args_command_line(argv,
                                       argv + sizeof(argv) / sizeof(argv[0]),
                                       regular_opts,
                                       args_opts);

    BOOST_CHECK(res);

    BOOST_REQUIRE(regular_opts.size() == 3);
    BOOST_CHECK(regular_opts[0] == "command");
    BOOST_CHECK(regular_opts[1] == "-arg1");
    BOOST_CHECK(regular_opts[2] == "--arg2");

    BOOST_REQUIRE(args_opts.size() == 2);
    BOOST_CHECK(args_opts[0] == "arg3");
    BOOST_CHECK(args_opts[1] == "--arg4");
}


/// Tests splitting empty command line
BOOST_AUTO_TEST_CASE(split_command_line_empty) {
    std::string cmd;
    std::istringstream str(cmd);

    std::vector<std::string> res;
    split_command_line(str, res);

    BOOST_CHECK(res.size() == 0);
}


/// Tests splitting command line with only spaces
BOOST_AUTO_TEST_CASE(split_command_line_spaces) {
    std::string cmd("\t  ");
    std::istringstream str(cmd);

    std::vector<std::string> res;
    split_command_line(str, res);

    BOOST_CHECK(res.size() == 0);
}


/// Tests splitting simple command line
BOOST_AUTO_TEST_CASE(split_command_line_simple) {
    std::string cmd("my_arg --zzz\t vvv");
    std::istringstream str(cmd);

    std::vector<std::string> res;
    split_command_line(str, res);

    BOOST_REQUIRE(res.size() == 3);
    BOOST_CHECK(res[0] == "my_arg");
    BOOST_CHECK(res[1] == "--zzz");
    BOOST_CHECK(res[2] == "vvv");
}


/// Tests splitting simple command line with quotes
BOOST_AUTO_TEST_CASE(split_command_line_quotes) {
    std::string cmd("my_arg \"--zz'z xxx\" vvv\"a a\"    'cccc\"nn'");
    std::istringstream str(cmd);

    std::vector<std::string> res;
    split_command_line(str, res);

    BOOST_REQUIRE(res.size() == 4);
    BOOST_CHECK(res[0] == "my_arg");
    BOOST_CHECK(res[1] == "--zz'z xxx");\
    BOOST_CHECK(res[2] == "vvva a");
    BOOST_CHECK(res[3] == "cccc\"nn");
}


/// Tests splitting simple command line with quotes and escape symbols
BOOST_AUTO_TEST_CASE(split_command_line_quotes_escape) {
    std::string cmd("my_arg \"--zz\\\"z xxx\" vv\\'v    'cc\\\\cc\"nn'");
    std::istringstream str(cmd);

    std::vector<std::string> res;
    split_command_line(str, res);

    BOOST_REQUIRE(res.size() == 4);
    BOOST_CHECK(res[0] == "my_arg");
    BOOST_CHECK(res[1] == "--zz\"z xxx");
    BOOST_CHECK(res[2] == "vv'v");
    BOOST_CHECK(res[3] == "cc\\cc\"nn");
}


/// Tests writting empty command line to output stream
BOOST_AUTO_TEST_CASE(write_command_line_empty) {
    std::vector<std::string> cmd;
    std::ostringstream str;

    write_command_line(str, cmd);
    std::string res = str.str();

    BOOST_CHECK(res.empty());
}


/// Tests writing command line with empty argument
BOOST_AUTO_TEST_CASE(write_command_line_empty_arg) {
    std::vector<std::string> cmd;
    cmd.push_back("");
    std::ostringstream str;

    write_command_line(str, cmd);
    std::string res = str.str();

    BOOST_CHECK(res == "\"\"");
}


/// Tests writing simple command line
BOOST_AUTO_TEST_CASE(write_command_line_simple) {
    std::vector<std::string> cmd;
    cmd.push_back("aaa");
    cmd.push_back("bbbb");
    std::ostringstream str;

    write_command_line(str, cmd);
    std::string res = str.str();

    BOOST_CHECK(res == "aaa bbbb");
}


/// Tests writing command line with spaces
BOOST_AUTO_TEST_CASE(write_command_line_spaces) {
    std::vector<std::string> cmd;
    cmd.push_back("aaa");
    cmd.push_back("bb bb");
    std::ostringstream str;

    write_command_line(str, cmd);
    std::string res = str.str();

    BOOST_CHECK(res == "aaa \"bb bb\"");
}



/// Tests writing command line with escape chars
BOOST_AUTO_TEST_CASE(write_command_line_escape) {
    std::vector<std::string> cmd;
    cmd.push_back("aaa\\");
    cmd.push_back("bb b\"b");
    cmd.push_back("cc'c'");
    std::ostringstream str;

    write_command_line(str, cmd);
    std::string res = str.str();

    BOOST_CHECK(res == "aaa\\\\ \"bb b\\\"b\" cc\\'c\\'");
}


BOOST_AUTO_TEST_SUITE_END()


} } }
