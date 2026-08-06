// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file regexp_lists_test.cpp 
/// Contains unit tests for the regexp_lists and regexp_list_item classes.

#include "../regex_lists.hpp"
#include "../regex_lists_defaults.hpp"
#include <boost/test/unit_test.hpp>
#include <iostream>


namespace cxxdbg::dbg::core::test {

    
BOOST_AUTO_TEST_SUITE(regexp_lists_test)


/// Tests assignment and comparison
BOOST_AUTO_TEST_CASE(check_assign_and_compare) {
    regex_lists lists1 = functions_to_skip::default_value();
    // list item
    regex_list_item l1("l1", true);
    l1.add("regexp1");
    l1.add("regexp2");
    l1.add("regexp3");
    
    // list item
    regex_list_item l2("l2", false);
    l2.add("regex4");
    l2.add("regex5");
    
    lists1.add(l1);
    lists1.add(l2);
    
    // another storage
    regex_lists lists2;    
    
    lists2 = lists1;
    
    BOOST_CHECK(lists1 == lists2);
}


/// Tests correctness of building regexps
BOOST_AUTO_TEST_CASE(check_combine_regexp) {
    regex_lists lists1;
    
    // list item
    regex_list_item l1("l1", true);
    l1.add("regexp1");
    l1.add("regexp2");
    l1.add("regexp3");
    
    // list item
    regex_list_item l2("l2", false);
    l2.add("regex4");
    l2.add("regex5");
    
    // list item
    regex_list_item l3("l3", true);
    l3.add("regexp6");
    
    
    lists1.add(l1);
    lists1.add(l2);
    lists1.add(l3);
    
    const std::string correct_value = "(regexp1)|(regexp2)|(regexp3)|(regexp6)";
    const std::string combined_value = lists1.combine_regex();
    
    //std::cout << "correct value = " << correct_value << std::endl;
    //std::cout << "combined value = " << combined_value << std::endl;
    
    BOOST_CHECK(correct_value == combined_value);
}


/// Tests comparison of two equal regex lists items with different enabled state
BOOST_AUTO_TEST_CASE(check_compare_enabled_disabled) {
    regex_list_item l1("l1", true);
    l1.add("regexp1");
    l1.add("regexp2");
    l1.add("regexp3");

    regex_list_item l2("l1", true);
    l2.add("regexp1");
    l2.add("regexp2");
    l2.add("regexp3");

    BOOST_CHECK(l1 == l2);
}


BOOST_AUTO_TEST_SUITE_END()


}
