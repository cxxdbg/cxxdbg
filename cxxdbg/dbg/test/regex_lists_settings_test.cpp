// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file regex_lists_settings_test.cpp
/// Contains unit tests saving/loading regex lists from/to settings.

#include "../regex_lists_settings.hpp"
#include "../core/regex_lists.hpp"
#include "cxxdbg/app/test/mock_settings_storage.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::test {


BOOST_AUTO_TEST_SUITE(regex_lists_settings)


/// Tests correctness of saving and loading options
BOOST_AUTO_TEST_CASE(check_save_load_and_compare) {
    mock_settings_storage settings;
    
    core::regex_lists lists1 = core::functions_to_step_into::default_value();
    core::regex_list_item l1("l1", true);
    l1.add("regexp1");
    l1.add("regexp2");
    l1.add("regexp3");
    
    core::regex_list_item l2("l2", false);
    l2.add("regex4");
    
    lists1.add(l1);
    
    lists1.add(l2);
    
    save_regex_lists_to_settings(settings, "mylist", lists1);
    
    core::regex_lists lists2 = load_regex_lists_from_settings(settings, "mylist", core::functions_to_step_into::default_value());
    
    BOOST_CHECK(lists1 == lists2);
}


BOOST_AUTO_TEST_SUITE_END()


}
