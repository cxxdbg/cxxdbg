// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file flat_tree_model_test.cpp
/// Contains unit tests for the flat_tree_model class.

#include "../flat_tree_model.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::test {


struct flat_tree_model_test_fixture {
    flat_tree_model<int> mdl;
};


BOOST_FIXTURE_TEST_SUITE(flat_tree_model_test, flat_tree_model_test_fixture)


/// Tests model construction
BOOST_AUTO_TEST_CASE(init) {
    BOOST_CHECK_EQUAL(mdl.childs_size(0), 0);
}


/// Tests adding nodes
BOOST_AUTO_TEST_CASE(add_nodes) {
    mdl.insert(0, 0, {1, 2, 3});
    mdl.insert(1, 0, {11, 12});

    BOOST_REQUIRE_EQUAL(mdl.childs_size(0), 3);
    BOOST_CHECK_EQUAL(mdl.child(0, 0), 1);
    BOOST_CHECK_EQUAL(mdl.child(0, 1), 2);
    BOOST_CHECK_EQUAL(mdl.child(0, 2), 3);

    BOOST_REQUIRE_EQUAL(mdl.childs_size(mdl.child(0, 0)), 2);
    BOOST_CHECK_EQUAL(mdl.child(mdl.child(0, 0), 0), 11);
    BOOST_CHECK_EQUAL(mdl.child(mdl.child(0, 0), 1), 12);
}


/// Tests removing nodes
BOOST_AUTO_TEST_CASE(remove_nodes) {
    mdl.insert(0, 0, {1, 2, 3});
    mdl.insert(1, 0, {11, 12});
    mdl.erase(1, 0, 0);
    mdl.insert(0, 0, {11});

    BOOST_REQUIRE_EQUAL(mdl.childs_size(0), 4);
    BOOST_CHECK_EQUAL(mdl.child(0, 0), 11);
    BOOST_CHECK_EQUAL(mdl.child(0, 1), 1);
    BOOST_CHECK_EQUAL(mdl.child(0, 2), 2);
    BOOST_CHECK_EQUAL(mdl.child(0, 3), 3);

    BOOST_REQUIRE_EQUAL(mdl.childs_size(mdl.child(0, 1)), 1);
    BOOST_CHECK_EQUAL(mdl.child(mdl.child(0, 1), 0), 12);
}



BOOST_AUTO_TEST_SUITE_END()


}
