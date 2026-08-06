// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file po_command_test.cpp
/// Contains unit tests for po_command class.

#include "cxxdbg/cli/po_command.hpp"
#include <boost/test/unit_test.hpp>


namespace po = boost::program_options;


namespace cxxdbg::dbg::cli::test {


BOOST_AUTO_TEST_SUITE(program_options_command_handler_test)


/// Simple test for program_options_command_handler
BOOST_AUTO_TEST_CASE(test_simple) {

    bool handler_called = false;
    auto handl = [&handler_called](const po::variables_map & cmd, const auto & chandler) {
        handler_called = true;

        BOOST_CHECK(cmd["myopt"].as<std::string>() == "my option value");
    };

    auto cmd = make_po_command(handl);
    cmd->opt_desc().add_options()("myopt", po::value<std::string>(), "my option");

    cmd->exec({"--myopt=my option value"}, [](const auto & r) {});

    BOOST_CHECK(handler_called);
}


/// Tests displaying help
BOOST_AUTO_TEST_CASE(test_help) {

    auto handl = [](const auto & cmd, const auto & chandler) {
    };

    auto handler = make_po_command([](const auto & ... args) {});
    handler->opt_desc().add_options()("myopt", po::value<std::string>(), "my option");
    handler->set_help_msg("helpmsg");

    bool handler_called = false;
    handler->help({}, [&handler_called](const std::string & res) {
        handler_called = true;
        std::string str{"helpmsg\n"
                        "\n"
                        "Command options:\n"
                        "  --myopt arg           my option\n"
                        "\n"};
        BOOST_CHECK(res == str);
    });

    BOOST_CHECK(handler_called);
}



BOOST_AUTO_TEST_SUITE_END()


}
