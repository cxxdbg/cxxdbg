// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file basic_tree_model_test.cpp
/// Contains unit tests for the basic_tree_model class.

#include "../basic_tree_model.hpp"
#include <boost/test/unit_test.hpp>
#include <memory>


namespace cxxdbg::test {


struct basic_tree_model_test_fixture {
    using node_type = basic_tree_model_node<int>;
    using node_up = std::unique_ptr<node_type>;
    using node_up_vector = std::vector<node_up>;
    basic_tree_model<node_type> mdl;
};


BOOST_FIXTURE_TEST_SUITE(basic_tree_model_test, basic_tree_model_test_fixture)


/// Tests model construction
BOOST_AUTO_TEST_CASE(init) {
    BOOST_CHECK_EQUAL(mdl.childs_size(nullptr), 0);
}


/// Tests inserting root nodes
BOOST_AUTO_TEST_CASE(insert_root) {

    int n_before_added_called = 0;
    mdl.before_added.connect([this, &n_before_added_called](auto parent, auto first, auto last) {
        ++n_before_added_called;
        BOOST_CHECK(parent == nullptr);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 2);

        BOOST_CHECK_EQUAL(mdl.childs_size(nullptr), 0);
    });

    int n_after_added_called = 0;
    mdl.after_added.connect([this, &n_after_added_called](auto parent, auto first, auto last) {
        ++n_after_added_called;
        BOOST_CHECK(parent == nullptr);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 2);

        BOOST_REQUIRE_EQUAL(mdl.childs_size(nullptr), 3);
        BOOST_CHECK_EQUAL(mdl.child(nullptr, 0)->value(), 10);
        BOOST_CHECK_EQUAL(mdl.child(nullptr, 1)->value(), 20);
        BOOST_CHECK_EQUAL(mdl.child(nullptr, 2)->value(), 30);
    });

    mdl.before_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.before_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    std::vector<std::unique_ptr<node_type>> nodes;
    nodes.push_back(std::make_unique<node_type>(10));
    nodes.push_back(std::make_unique<node_type>(20));
    nodes.push_back(std::make_unique<node_type>(30));
    mdl.insert_nodes(nullptr, 0, nodes | std::ranges::views::all);

    BOOST_CHECK_EQUAL(n_before_added_called, 1);
    BOOST_CHECK_EQUAL(n_after_added_called, 1);

    BOOST_REQUIRE_EQUAL(mdl.childs_size(nullptr), 3);
    BOOST_CHECK_EQUAL(mdl.child(nullptr, 0)->value(), 10);
    BOOST_CHECK_EQUAL(mdl.child(nullptr, 1)->value(), 20);
    BOOST_CHECK_EQUAL(mdl.child(nullptr, 2)->value(), 30);

    BOOST_CHECK(mdl.parent(mdl.child(nullptr, 0)) == nullptr);
    BOOST_CHECK(mdl.parent(mdl.child(nullptr, 1)) == nullptr);
    BOOST_CHECK(mdl.parent(mdl.child(nullptr, 2)) == nullptr);

    BOOST_CHECK_EQUAL(mdl.index(mdl.child(nullptr, 0)), 0);
    BOOST_CHECK_EQUAL(mdl.index(mdl.child(nullptr, 1)), 1);
    BOOST_CHECK_EQUAL(mdl.index(mdl.child(nullptr, 2)), 2);
}


/// Tests inserting empty range of root nodes
BOOST_AUTO_TEST_CASE(insert_root_empty) {

    mdl.before_added.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_added.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.before_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.before_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    node_up_vector nodes;
    mdl.insert_nodes(nullptr, 0, nodes | std::ranges::views::all);

    BOOST_REQUIRE_EQUAL(mdl.childs_size(nullptr), 0);
}


/// Tests inserting child nodes
BOOST_AUTO_TEST_CASE(insert_childs) {

    std::vector<std::unique_ptr<node_type>> nodes;
    nodes.push_back(std::make_unique<node_type>(10));
    nodes.push_back(std::make_unique<node_type>(20));
    nodes.push_back(std::make_unique<node_type>(30));
    mdl.insert_nodes(nullptr, 0, nodes | std::ranges::views::all);

    BOOST_REQUIRE_EQUAL(mdl.childs_size(nullptr), 3);

    int n_before_added_called = 0;
    mdl.before_added.connect([this, &n_before_added_called](auto parent, auto first, auto last) {
        ++n_before_added_called;
        BOOST_CHECK(parent == mdl.child(nullptr, 0));
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 1);

        BOOST_CHECK_EQUAL(mdl.childs_size(mdl.child(nullptr, 0)), 0);
    });

    int n_after_added_called = 0;
    mdl.after_added.connect([this, &n_after_added_called](auto parent, auto first, auto last) {
        ++n_after_added_called;
        BOOST_CHECK(parent == mdl.child(nullptr, 0));
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 1);

        auto root = mdl.child(nullptr, 0);
        BOOST_REQUIRE_EQUAL(mdl.childs_size(root), 2);
        BOOST_CHECK_EQUAL(mdl.child(root, 0)->value(), 300);
        BOOST_CHECK_EQUAL(mdl.child(root, 1)->value(), 400);
        BOOST_CHECK(mdl.parent(mdl.child(root, 0)) == root);
        BOOST_CHECK(mdl.parent(mdl.child(root, 1)) == root);
        BOOST_CHECK_EQUAL(mdl.index(mdl.child(root, 0)), 0);
        BOOST_CHECK_EQUAL(mdl.index(mdl.child(root, 1)), 1);
    });

    mdl.before_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.before_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    auto root = mdl.child(nullptr, 0);
    node_up_vector nodes2;
    nodes2.push_back(std::make_unique<node_type>(300));
    nodes2.push_back(std::make_unique<node_type>(400));
    mdl.insert_nodes(root, 0, nodes2 | std::ranges::views::all);

    BOOST_CHECK_EQUAL(n_before_added_called, 1);
    BOOST_CHECK_EQUAL(n_after_added_called, 1);

    BOOST_REQUIRE_EQUAL(mdl.childs_size(root), 2);
    BOOST_CHECK_EQUAL(mdl.child(root, 0)->value(), 300);
    BOOST_CHECK_EQUAL(mdl.child(root, 1)->value(), 400);
    BOOST_CHECK(mdl.parent(mdl.child(root, 0)) == root);
    BOOST_CHECK(mdl.parent(mdl.child(root, 1)) == root);
    BOOST_CHECK_EQUAL(mdl.index(mdl.child(root, 0)), 0);
    BOOST_CHECK_EQUAL(mdl.index(mdl.child(root, 1)), 1);
}


/// Tests inserting empty range of child nodes
BOOST_AUTO_TEST_CASE(insert_childs_empty) {

    std::vector<std::unique_ptr<node_type>> nodes;
    nodes.push_back(std::make_unique<node_type>(10));
    nodes.push_back(std::make_unique<node_type>(20));
    nodes.push_back(std::make_unique<node_type>(30));
    mdl.insert_nodes(nullptr, 0, nodes | std::ranges::views::all);

    BOOST_REQUIRE_EQUAL(mdl.childs_size(nullptr), 3);

    mdl.before_added.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_added.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.before_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.before_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    auto root = mdl.child(nullptr, 0);
    node_up_vector empty;
    mdl.insert_nodes(root, 0, empty | std::ranges::views::all);

    BOOST_REQUIRE_EQUAL(mdl.childs_size(root), 0);
}


/// Tests removing root nodes
BOOST_AUTO_TEST_CASE(remove_root) {
    std::vector<std::unique_ptr<node_type>> nodes;
    nodes.push_back(std::make_unique<node_type>(100));
    nodes.push_back(std::make_unique<node_type>(200));
    nodes.push_back(std::make_unique<node_type>(300));
    mdl.insert_nodes(nullptr, 0, nodes | std::ranges::views::all);

    {
        node_up_vector nodes2;
        nodes2.push_back(std::make_unique<node_type>(101));
        nodes2.push_back(std::make_unique<node_type>(102));
        mdl.insert_nodes(mdl.child(nullptr, 0), 0, nodes2 | std::ranges::views::all);
    }
    {
        node_up_vector nodes2;
        nodes2.push_back(std::make_unique<node_type>(201));
        nodes2.push_back(std::make_unique<node_type>(202));
        mdl.insert_nodes(mdl.child(nullptr, 1), 0, nodes2 | std::ranges::views::all);
    }
    {
        node_up_vector nodes2;
        nodes2.push_back(std::make_unique<node_type>(301));
        nodes2.push_back(std::make_unique<node_type>(302));
        mdl.insert_nodes(mdl.child(nullptr, 2), 0, nodes2 | std::ranges::views::all);
    }


    mdl.before_added.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_added.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    int n_before_removed_called = 0;
    mdl.before_removed.connect([this, &n_before_removed_called](auto parent, auto first, auto last) {
        ++n_before_removed_called;

        BOOST_CHECK(parent == nullptr);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 1);

        BOOST_REQUIRE_EQUAL(mdl.childs_size(nullptr), 3);
    });

    int n_after_removed_called = 0;
    mdl.after_removed.connect([this, &n_after_removed_called](auto parent, auto first, auto last) {
        ++n_after_removed_called;

        BOOST_CHECK(parent == nullptr);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 1);

        BOOST_REQUIRE_EQUAL(mdl.childs_size(nullptr), 1);
        BOOST_CHECK_EQUAL(mdl.child(nullptr, 0)->value(), 300);
        BOOST_REQUIRE_EQUAL(mdl.childs_size(mdl.child(nullptr, 0)), 2);
        BOOST_CHECK_EQUAL(mdl.child(mdl.child(nullptr, 0), 0)->value(), 301);
        BOOST_CHECK_EQUAL(mdl.child(mdl.child(nullptr, 0), 1)->value(), 302);
    });

    mdl.before_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    auto rnodes = mdl.erase(nullptr, 0, 1);

    BOOST_CHECK_EQUAL(n_before_removed_called, 1);
    BOOST_CHECK_EQUAL(n_after_removed_called, 1);

    BOOST_REQUIRE_EQUAL(mdl.childs_size(nullptr), 1);
    BOOST_CHECK_EQUAL(mdl.child(nullptr, 0)->value(), 300);
    BOOST_REQUIRE_EQUAL(mdl.childs_size(mdl.child(nullptr, 0)), 2);
    BOOST_CHECK_EQUAL(mdl.child(mdl.child(nullptr, 0), 0)->value(), 301);
    BOOST_CHECK_EQUAL(mdl.child(mdl.child(nullptr, 0), 1)->value(), 302);

    BOOST_REQUIRE_EQUAL(rnodes.size(), 2);
    BOOST_CHECK_EQUAL(rnodes[0]->value(), 100);
    BOOST_CHECK_EQUAL(rnodes[1]->value(), 200);
}


/// Tests removing child nodes
BOOST_AUTO_TEST_CASE(remove_childs) {
    std::vector<std::unique_ptr<node_type>> nodes;
    nodes.push_back(std::make_unique<node_type>(100));
    nodes.push_back(std::make_unique<node_type>(200));
    nodes.push_back(std::make_unique<node_type>(300));
    mdl.insert_nodes(nullptr, 0, nodes | std::ranges::views::all);

    {
        node_up_vector nodes2;
        nodes2.push_back(std::make_unique<node_type>(101));
        nodes2.push_back(std::make_unique<node_type>(102));
        mdl.insert_nodes(mdl.child(nullptr, 0), 0, nodes2 | std::ranges::views::all);
    }
    {
        node_up_vector nodes2;
        nodes2.push_back(std::make_unique<node_type>(201));
        nodes2.push_back(std::make_unique<node_type>(202));
        mdl.insert_nodes(mdl.child(nullptr, 1), 0, nodes2 | std::ranges::views::all);
    }
    {
        node_up_vector nodes2;
        nodes2.push_back(std::make_unique<node_type>(301));
        nodes2.push_back(std::make_unique<node_type>(302));
        mdl.insert_nodes(mdl.child(nullptr, 2), 0, nodes2 | std::ranges::views::all);
    }

    mdl.before_added.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_added.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    int n_before_removed_called = 0;
    mdl.before_removed.connect([this, &n_before_removed_called](auto parent, auto first, auto last) {
        ++n_before_removed_called;

        BOOST_REQUIRE_EQUAL(mdl.childs_size(nullptr), 3);
        BOOST_CHECK_EQUAL(mdl.childs_size(mdl.child(nullptr, 1)), 2);
    });

    int n_after_removed_called = 0;
    mdl.after_removed.connect([this, &n_after_removed_called](auto parent, auto first, auto last) {
        ++n_after_removed_called;

        BOOST_REQUIRE_EQUAL(mdl.childs_size(nullptr), 3);
        BOOST_CHECK_EQUAL(mdl.child(nullptr, 1)->value(), 200);
        BOOST_REQUIRE_EQUAL(mdl.childs_size(mdl.child(nullptr, 1)), 1);
        BOOST_CHECK_EQUAL(mdl.child(mdl.child(nullptr, 1), 0)->value(), 202);
    });

    mdl.before_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    auto rnodes = mdl.erase(mdl.child(nullptr, 1), 0, 0);

    BOOST_CHECK_EQUAL(n_before_removed_called, 1);
    BOOST_CHECK_EQUAL(n_after_removed_called, 1);

    BOOST_REQUIRE_EQUAL(mdl.childs_size(nullptr), 3);
    BOOST_CHECK_EQUAL(mdl.child(nullptr, 1)->value(), 200);
    BOOST_REQUIRE_EQUAL(mdl.childs_size(mdl.child(nullptr, 1)), 1);
    BOOST_CHECK_EQUAL(mdl.child(mdl.child(nullptr, 1), 0)->value(), 202);

    BOOST_REQUIRE_EQUAL(rnodes.size(), 1);
    BOOST_CHECK_EQUAL(rnodes[0]->value(), 201);
}


BOOST_AUTO_TEST_SUITE_END()


}
