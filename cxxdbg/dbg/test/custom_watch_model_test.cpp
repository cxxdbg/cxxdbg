// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file custom_watch_model_test.cpp
/// Contains unit tests for the custom_watch_model class.

#include "mock_custom_watch_list_impl.hpp"
#include "cxxdbg/dbg/custom_watch_list.hpp"
#include "cxxdbg/dbg/custom_watch_model.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::test {


struct custom_watch_model_test_fixture {
    mock_custom_watch_list_impl impl;
    custom_watch_list watch{&impl};
    custom_watch_model model{watch};
};


BOOST_FIXTURE_TEST_SUITE(custom_watch_model_test, custom_watch_model_test_fixture)


/// Tests adding new item to model
BOOST_AUTO_TEST_CASE(test_add) {
    MOCK_ADD_CALL(impl, add_watch, [](const auto & expr, auto && node_info) {
        BOOST_CHECK(expr == L"var");
    });
    model.add(L"var");

    BOOST_REQUIRE(model.childs_size() == 1);
    auto root = model.child({}, 0);
    BOOST_CHECK(model.text(root, 0) == L"var");
    BOOST_CHECK(model.text(root, 1) == L"");
    BOOST_CHECK(model.text(root, 2) == L"");

    BOOST_CHECK(impl.verify());
}


/// Tests removing watch
BOOST_AUTO_TEST_CASE(remove_watch) {
    // adding empty item
    MOCK_ADD_CALL(impl, add_watch, [](const auto & expr, auto && node_info) {
        BOOST_CHECK(expr == L"var");
    });
    model.add(L"var");

    // notifying watch list about adding new watch
    {
        watch_list_impl::tree_info tree;
        tree.add_node({10, L"var", L"val", L"type", 0});
        impl.emit_tree_updated_signal(tree);
    }

    // removing watch. Expecting call to remove_watch in implementation
    MOCK_ADD_CALL(impl, remove_watch, [](auto id) {
        BOOST_CHECK(id == 10);
    });
    model.remove(0);

    BOOST_CHECK(model.childs_size() == 0);
    BOOST_CHECK(impl.verify());
}


/// Tests removing by setting empty string
BOOST_AUTO_TEST_CASE(remove_watch_set) {
    // adding empty item
    MOCK_ADD_CALL(impl, add_watch, [](const auto & expr, auto && node_info) {
        BOOST_CHECK(expr == L"var");
    });
    model.add(L"var");

    // notifying watch list about adding new watch
    {
        watch_list_impl::tree_info tree;
        tree.add_node({10, L"var", L"val", L"type", 0});
        impl.emit_tree_updated_signal(tree);
    }

    // removing watch. Expecting call to remove_watch in implementation
    MOCK_ADD_CALL(impl, remove_watch, [](auto id) {
        BOOST_CHECK(id == 10);
    });
    model.set_text(model.child({}, 0), 0, L"");

    BOOST_CHECK(model.childs_size() == 0);
    BOOST_CHECK(impl.verify());
}



/// Tests updating watch
BOOST_AUTO_TEST_CASE(set_watch) {
    // adding empty item
    MOCK_ADD_CALL(impl, add_watch, [](const auto & expr, auto && node_info) {
        BOOST_CHECK(expr == L"var");
    });
    model.add(L"var");

    // notifying watch list about adding new watch
    {
        watch_list_impl::tree_info tree;
        tree.add_node({10, L"var", L"val", L"type", 0});
        impl.emit_tree_updated_signal(tree);
    }

    // setting text for new item. expecting call to set_watch
    // in implementation
    BOOST_REQUIRE(model.childs_size() == 1);
    MOCK_ADD_CALL(impl, set_watch, [](auto id, const auto & expr, auto && node_info) {
        BOOST_CHECK(id == 10);
        BOOST_CHECK(expr == L"var2");
    });
    model.set_text(model.child({}, 0), 0, L"var2");

    BOOST_CHECK(impl.verify());
}



BOOST_AUTO_TEST_SUITE_END()


}
