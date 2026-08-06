// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file template_test.cpp
/// Contains unit tests for functions in template.cpp

#include "cxxdbg/util/template.hpp"
#include <boost/test/unit_test.hpp>
#include <sstream>


namespace cxxdbg { namespace util { namespace test {


BOOST_AUTO_TEST_SUITE(template_test)


/// Tests parsing non-template class
BOOST_AUTO_TEST_CASE(nontempl) {
    std::istringstream str("my_class");
    std::string name;
    std::vector<std::string> pars;
    parse_template_name(str, name, pars);

    BOOST_CHECK(name == "my_class");
    BOOST_CHECK(pars.empty());
}


/// Tests parsing simple template
BOOST_AUTO_TEST_CASE(simple_templ) {
    std::istringstream str("my_templ<asd, sdsd, a>");
    std::string name;
    std::vector<std::string> pars;
    parse_template_name(str, name, pars);

    BOOST_CHECK(name == "my_templ");

    BOOST_REQUIRE(pars.size() == 3);
    BOOST_CHECK(pars[0] == "asd");
    BOOST_CHECK(pars[1] == "sdsd");
    BOOST_CHECK(pars[2] == "a");
}


/// Tests parsing non-template class in template scope
BOOST_AUTO_TEST_CASE(simple_nested_nontempl) {
    std::istringstream str("my_templ<asd, sdsd, a>::nested");
    std::string name;
    std::vector<std::string> pars;
    parse_template_name(str, name, pars);
    BOOST_CHECK(name == "my_templ<asd, sdsd, a>::nested");
    BOOST_CHECK(pars.empty());
}


/// Tests parsing simple template in template scope
BOOST_AUTO_TEST_CASE(simple_templ_templ_scope) {
    std::istringstream str("my_templ<asd, sdsd, a>::t2<x, 23>");
    std::string name;
    std::vector<std::string> pars;
    parse_template_name(str, name, pars);

    BOOST_CHECK(name == "my_templ<asd, sdsd, a>::t2");
    BOOST_REQUIRE(pars.size() == 2);
    BOOST_CHECK(pars[0] == "x");
    BOOST_CHECK(pars[1] == "23");
}


/// Tests parsin template in scope
BOOST_AUTO_TEST_CASE(templ_in_scope) {
    std::istringstream str("sss::t2<x, 23>");
    std::string name;
    std::vector<std::string> pars;
    parse_template_name(str, name, pars);

    BOOST_CHECK(name == "sss::t2");
    BOOST_REQUIRE(pars.size() == 2);
    BOOST_CHECK(pars[0] == "x");
    BOOST_CHECK(pars[1] == "23");
}


/// Tests parsing template in template scope with :: in parameters
BOOST_AUTO_TEST_CASE(templ_scope_nested) {
    std::istringstream str("sss<aaa::b<10, 20>>::t2<x, 23>");
    std::string name;
    std::vector<std::string> pars;
    parse_template_name(str, name, pars);

    BOOST_CHECK(name == "sss<aaa::b<10, 20>>::t2");
    BOOST_REQUIRE(pars.size() == 2);
    BOOST_CHECK(pars[0] == "x");
    BOOST_CHECK(pars[1] == "23");
}


/// Tests parsing template with template parameter
BOOST_AUTO_TEST_CASE(templ_templ_par) {
    std::istringstream str("t2<xx::aa<T>, 23>");
    std::string name;
    std::vector<std::string> pars;
    parse_template_name(str, name, pars);

    BOOST_CHECK(name == "t2");
    BOOST_REQUIRE(pars.size() == 2);
    BOOST_CHECK(pars[0] == "xx::aa<T>");
    BOOST_CHECK(pars[1] == "23");
}


BOOST_AUTO_TEST_SUITE_END()


} } }
