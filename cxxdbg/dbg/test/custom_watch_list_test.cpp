// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file custom_watch_list.cpp
/// Contains unit tests for the custom_watch_list class.

#include "mock_custom_watch_list_impl.hpp"
#include "cxxdbg/dbg/custom_watch_list.hpp"
#include "cxxdbg/dbg/watch_list.hpp"
#include "cxxdbg/dbg/watch_list_impl.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::test {


struct custom_watch_list_test_fixture {
    mock_custom_watch_list_impl impl;
    custom_watch_list watch{&impl};
};


BOOST_FIXTURE_TEST_SUITE(custom_watch_list_test, custom_watch_list_test_fixture)


/// Tests adding value to watch list
BOOST_AUTO_TEST_CASE(add_watch) {
    MOCK_ADD_CALL(impl, add_watch, [](const auto & expr, auto && handl) {
        BOOST_CHECK(expr == L"my expr");
    });

    watch.add_watch(L"my expr");

    // watch node should be immediatelly added in watch list
    BOOST_REQUIRE(watch.root_nodes_size() == 1);
    auto it = watch.root_nodes_begin();
    BOOST_REQUIRE(it != watch.root_nodes_end());
    auto node = *it;
    BOOST_REQUIRE(node);

    BOOST_CHECK(node->name() == L"my expr");
    BOOST_CHECK(node->value() == L"");
    BOOST_CHECK(node->type() == L"");
    BOOST_CHECK(node->childs_size() == 0);

    BOOST_CHECK(impl.verify());
}


/// Tests updating watch node after receiving it from impl
BOOST_AUTO_TEST_CASE(add_watch_update) {
    watch_list_impl::node_handler handl;

    MOCK_ADD_CALL(impl, add_watch, [&handl](const auto & expr, auto && h) {
        BOOST_CHECK(expr == L"my expr");
        handl = h;
    });
    watch.add_watch(L"my expr");

    // watch node should be immediatelly added in watch list
    BOOST_REQUIRE(watch.root_nodes_size() == 1);
    auto node = *watch.root_nodes_begin();
    BOOST_REQUIRE(node);

    // sending node to watch list
    {
        watch_list_impl_tree_info_node node {10, L"my expr", L"my val", L"my type", 0};
        handl(node);
    }

    // checking that root node was not changed
    BOOST_REQUIRE(watch.root_nodes_size() == 1);
    auto node2 = *watch.root_nodes_begin();
    BOOST_CHECK(node2 == node);

    // checking new node values
    BOOST_CHECK(node2->name() == L"my expr");
    BOOST_CHECK(node2->value() == L"my val");
    BOOST_CHECK(node2->type() == L"my type");
    BOOST_CHECK(node2->childs_size() == 0);

    BOOST_CHECK(impl.verify());
}


/// Tests removing watch with known ID
BOOST_AUTO_TEST_CASE(remove_watch) {
    // adding watch
    MOCK_ADD_CALL(impl, add_watch, [](const auto & expr, auto && h) {
        BOOST_CHECK(expr == L"my expr");

        // sending node to watch list
        h(watch_list_impl_tree_info_node{10, L"my expr", L"my val", L"my type", 0});
    });
    watch.add_watch(L"my expr");

    // removing watch
    BOOST_REQUIRE(watch.root_nodes_begin() != watch.root_nodes_end());
    MOCK_ADD_CALL(impl, remove_watch, [](auto id) {
        BOOST_CHECK(id == 10);
    });
    watch.remove_watch(*watch.root_nodes_begin());

    // checking that watch list is empty
    BOOST_CHECK(watch.root_nodes_begin() == watch.root_nodes_end());

    BOOST_CHECK(impl.verify());
}


/// Tests removing watch before update is received
BOOST_AUTO_TEST_CASE(test_remove_no_id) {
    watch_list_impl::node_handler handl;

    // adding watch
    MOCK_ADD_CALL(impl, add_watch, [&handl](const auto & expr, auto && h) {
        BOOST_CHECK(expr == L"my expr");
        handl = h;
    });
    watch.add_watch(L"my expr");

    // removing watch. watch list should not send request to impl
    watch.remove_watch(*watch.root_nodes_begin());
    BOOST_REQUIRE(watch.root_nodes_begin() == watch.root_nodes_end());

    // sending node info to watch list. After receiving id for deleted
    // watch, watch list should send delete request to impl
    MOCK_ADD_CALL(impl, remove_watch, [](auto id) {
        BOOST_CHECK(id == 10);
    });

    handl(watch_list_impl_tree_info_node{10, L"my expr", L"my val", L"my type", 0});

    BOOST_REQUIRE(watch.root_nodes_begin() == watch.root_nodes_end());

    BOOST_CHECK(impl.verify());
}


/// Tests setting expression for watch with known ID
BOOST_AUTO_TEST_CASE(set_watch) {
    // adding watch
    MOCK_ADD_CALL(impl, add_watch, [](const auto & expr, auto && h) {
        BOOST_CHECK(expr == L"my expr");

        // sending node to watch list
        h(watch_list_impl_tree_info_node{10, L"my expr", L"my val", L"my type", 0});
    });
    watch.add_watch(L"my expr");

    // setting watch expression
    BOOST_REQUIRE(watch.root_nodes_begin() != watch.root_nodes_end());
    MOCK_ADD_CALL(impl, set_watch, [](auto id, const auto & expr, auto && h) {
        BOOST_CHECK(id == 10);
        BOOST_CHECK(expr == L"e2");
    });
    watch.set_watch(*watch.root_nodes_begin(), L"e2");

    // checking that watch list contains updated expression
    BOOST_CHECK(watch.root_nodes_begin() != watch.root_nodes_end());
    auto node = *watch.root_nodes_begin();
    BOOST_CHECK(node);
    BOOST_CHECK(node->name() == L"e2");
    BOOST_CHECK(node->value() == L"");
    BOOST_CHECK(node->type() == L"");
    BOOST_CHECK(node->childs_size() == 0);
}


/// Tests setting watch expression before update is received
BOOST_AUTO_TEST_CASE(test_set_no_id) {
    watch_list_impl::node_handler handl;

    // adding watch
    MOCK_ADD_CALL(impl, add_watch, [&handl](const auto & expr, auto && h) {
        BOOST_CHECK(expr == L"my expr");
        handl = h;
    });
    watch.add_watch(L"my expr");

    // setting watch expression. watch list should not send request to impl
    watch.set_watch(*watch.root_nodes_begin(), L"e2");

    // checking watch list
    BOOST_REQUIRE(watch.root_nodes_begin() != watch.root_nodes_end());
    auto node = *watch.root_nodes_begin();
    BOOST_REQUIRE(node);
    BOOST_CHECK(node->name() == L"e2");
    BOOST_CHECK(node->value() == L"");
    BOOST_CHECK(node->type() == L"");

    // sending node info to watch list. After receiving id for changed
    // watch, watch list should send update request to impl
    MOCK_ADD_CALL(impl, set_watch, [](auto id, const auto & expr, auto && node_info) {
        BOOST_CHECK(id == 10);
        BOOST_CHECK(expr == L"e2");
    });

    handl(watch_list_impl_tree_info_node{10, L"my expr", L"my val", L"my type", 0});

    // checking watch list
    BOOST_REQUIRE(watch.root_nodes_begin() != watch.root_nodes_end());
    node = *watch.root_nodes_begin();
    BOOST_REQUIRE(node);
    BOOST_CHECK(node->name() == L"e2");
    BOOST_CHECK(node->value() == L"");
    BOOST_CHECK(node->type() == L"");
    BOOST_CHECK(node->childs_size() == 0);

    BOOST_CHECK(impl.verify());
}


/// Tests expanding node in custom watch list
BOOST_AUTO_TEST_CASE(test_expand) {
    // adding single root node with 2 childs to watch list
    MOCK_ADD_CALL(impl, add_watch, [](const auto & expr, auto && h) {
        BOOST_CHECK(expr == L"var");
        h(watch_list_impl_tree_info_node{13, L"var", L"my val", L"my type", 2});
    });
    watch.add_watch(L"var");

    // expanding node
    MOCK_ADD_CALL(impl, expand_node, ([](auto id, auto && handl) {
        BOOST_CHECK(id == 13);

        watch_list_impl::tree_info::node n{13, L"var", L"my val", L"my type", 2};
        n.add_child({15, L"c1", L"c1 val", L"c1 type", 0});
        n.add_child({17, L"c2", L"c2 val", L"c2 type", 0});

        handl(n);
    }));
    BOOST_REQUIRE(watch.root_nodes_begin() != watch.root_nodes_end());
    watch.expand_node(*watch.root_nodes_begin());

    // checking watch list
    BOOST_REQUIRE(watch.root_nodes_begin() != watch.root_nodes_end());
    auto node = *watch.root_nodes_begin();
    BOOST_REQUIRE(node);

    BOOST_CHECK(node->name() == L"var");
    BOOST_CHECK(node->value() == L"my val");
    BOOST_CHECK(node->type() == L"my type");

    BOOST_REQUIRE(node->childs_size() == 2);

    auto child1 = node->child_at(0);
    BOOST_REQUIRE(child1);
    BOOST_CHECK(child1->name() == L"c1");
    BOOST_CHECK(child1->value() == L"c1 val");
    BOOST_CHECK(child1->type() == L"c1 type");
    BOOST_CHECK(child1->childs_size() == 0);

    auto child2 = node->child_at(1);
    BOOST_REQUIRE(child2);
    BOOST_CHECK(child2->name() == L"c2");
    BOOST_CHECK(child2->value() == L"c2 val");
    BOOST_CHECK(child2->type() == L"c2 type");
    BOOST_CHECK(child2->childs_size() == 0);

    BOOST_CHECK(impl.verify());
}


/// Tests resetting implementation
BOOST_AUTO_TEST_CASE(reset_impl) {
    // adding first root node to watch list
    MOCK_ADD_CALL(impl, add_watch, [](const auto & expr, auto && h) {
        BOOST_CHECK(expr == L"var");
        h(watch_list_impl::tree_info::node{10, L"var", L"my val", L"my type", 2});
    });
    watch.add_watch(L"var");

    // adding second root node to watch list
    MOCK_ADD_CALL(impl, add_watch, [](const auto & expr, auto && h) {
        BOOST_CHECK(expr == L"var2");
        h(watch_list_impl::tree_info::node{20, L"var2", L"my val 2", L"my type 2", 0});
    });
    watch.add_watch(L"var2");


    BOOST_REQUIRE_EQUAL(watch.root_nodes_size(), 2);

    const watch_list_tree_node * node1 = nullptr, * node2 = nullptr;
    {
        auto it = watch.root_nodes_begin();
        BOOST_REQUIRE(it != watch.root_nodes_end());
        node1 = *it;

        ++it;
        BOOST_REQUIRE(it != watch.root_nodes_end());
        node2 = *it;
    }

    // Resetting implementation. Watch list should remove values from all root nodes

    watch.connect_root_node_removed([](auto && ...) {
        BOOST_CHECK(false);
    });

    watch.connect_root_node_added([](auto && ...) {
        BOOST_CHECK(false);
    });

    bool n1_changed_called = false;
    bool n2_changed_called = false;
    watch.connect_node_changed([&n1_changed_called, &n2_changed_called, node1, node2](const watch_list_tree_node * node) {
        if (node == node1) {
            BOOST_CHECK(!n1_changed_called);
            n1_changed_called = true;

            BOOST_CHECK(node->name() == L"var");
            BOOST_CHECK(node->value() == L"");
            BOOST_CHECK(node->type() == L"");
            BOOST_CHECK_EQUAL(node->childs_size(), 2);
        } else if (node == node2) {
            BOOST_CHECK(!n2_changed_called);
            n2_changed_called = true;

            BOOST_CHECK(node->name() == L"var2");
            BOOST_CHECK(node->value() == L"");
            BOOST_CHECK(node->type() == L"");
            BOOST_CHECK_EQUAL(node->childs_size(), 0);
        } else {
            BOOST_CHECK(false);
        }
    });

    watch.connect_before_added([](auto && ...) {
        BOOST_CHECK(false);
    });

    watch.connect_after_added([](auto && ...) {
        BOOST_CHECK(false);
    });

    bool before_removed_called = false;
    watch.connect_before_removed([&before_removed_called, node1](auto node, size_t first, size_t last) {
        BOOST_CHECK(!before_removed_called);
        before_removed_called = true;

        BOOST_CHECK(node == node1);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 1);
        BOOST_CHECK_EQUAL(node->childs_size(), 2);
    });

    bool after_removed_called = false;
    watch.connect_after_removed([&after_removed_called, node1](auto node, size_t first, size_t last) {
        BOOST_CHECK(!after_removed_called);
        after_removed_called = true;

        BOOST_CHECK(node == node1);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 1);
        BOOST_CHECK_EQUAL(node->childs_size(), 0);
    });

    watch.set_impl(nullptr);

    BOOST_CHECK(n1_changed_called);
    BOOST_CHECK(n2_changed_called);
    BOOST_CHECK(before_removed_called);
    BOOST_CHECK(after_removed_called);


    // checking result tree

    BOOST_REQUIRE_EQUAL(watch.root_nodes_size(), 2);
    {
        auto it = watch.root_nodes_begin();
        BOOST_REQUIRE(it != watch.root_nodes_end());
        node1 = *it;

        ++it;
        BOOST_REQUIRE(it != watch.root_nodes_end());
        node2 = *it;
    }

    BOOST_CHECK(node1->name() == L"var");
    BOOST_CHECK(node1->value() == L"");
    BOOST_CHECK(node1->type() == L"");
    BOOST_CHECK_EQUAL(node1->childs_size(), 0);

    BOOST_CHECK(node2->name() == L"var2");
    BOOST_CHECK(node2->value() == L"");
    BOOST_CHECK(node2->type() == L"");
    BOOST_CHECK_EQUAL(node2->childs_size(), 0);

    BOOST_CHECK(impl.verify());
}


/// Tests setting implementation with existing watches
BOOST_AUTO_TEST_CASE(set_new_impl) {
    // removing implementation
    watch.set_impl(nullptr);

    // adding watches
    watch.add_watch(L"var");
    watch.add_watch(L"var2");

    // checking that watch tree contains empty values

    const watch_list_tree_node * node1 = nullptr, * node2 = nullptr;
    {
        BOOST_REQUIRE_EQUAL(watch.root_nodes_size(), 2);

        auto it = watch.root_nodes_begin();
        BOOST_REQUIRE(it != watch.root_nodes_end());
        node1 = *it;

        ++it;
        BOOST_REQUIRE(it != watch.root_nodes_end());
        node2 = *it;
    }

    BOOST_CHECK(node1->name() == L"var");
    BOOST_CHECK(node1->value() == L"");
    BOOST_CHECK(node1->type() == L"");
    BOOST_CHECK_EQUAL(node1->childs_size(), 0);

    BOOST_CHECK(node2->name() == L"var2");
    BOOST_CHECK(node2->value() == L"");
    BOOST_CHECK(node2->type() == L"");
    BOOST_CHECK_EQUAL(node2->childs_size(), 0);


    // setting implementation. Watch list should call add_watch in implementation for each watch

    watch_list_impl::node_handler var_handler, var2_handler;

    bool set_fmt_opts_called = false;
    MOCK_ADD_CALL(impl, set_fmt_opts, [&set_fmt_opts_called](auto && opts, auto && h) {
        BOOST_CHECK(!set_fmt_opts_called);
        set_fmt_opts_called = true;
    });

    MOCK_ADD_CALL(impl, add_watch, [&var_handler](const auto & expr, auto && h) {
        BOOST_CHECK(expr == L"var");
        var_handler = h;
    });

    MOCK_ADD_CALL(impl, add_watch, [&var2_handler](const auto & expr, auto && h) {
        BOOST_CHECK(expr == L"var2");
        var2_handler = h;
    });

    watch.set_impl(&impl);

    BOOST_CHECK(set_fmt_opts_called);

    // sending result to watch list for each add_watch call. Watch list should update values

    watch.connect_root_node_removed([](auto && ...) {
        BOOST_CHECK(false);
    });

    watch.connect_root_node_added([](auto && ...) {
        BOOST_CHECK(false);
    });

    bool n1_changed_called = false;
    bool n2_changed_called = false;
    watch.connect_node_changed([&n1_changed_called, &n2_changed_called, node1, node2](const watch_list_tree_node * node) {
        if (node == node1) {
//            BOOST_CHECK(!n1_changed_called);
            n1_changed_called = true;

            BOOST_CHECK(node->name() == L"var");
            BOOST_CHECK(node->value() == L"my val");
            BOOST_CHECK(node->type() == L"my type");
//            BOOST_CHECK_EQUAL(node->childs_size(), 0);
        } else if (node == node2) {
//            BOOST_CHECK(!n2_changed_called);
            n2_changed_called = true;

            BOOST_CHECK(node->name() == L"var2");
            BOOST_CHECK(node->value() == L"my val 2");
            BOOST_CHECK(node->type() == L"my type 2");
//            BOOST_CHECK_EQUAL(node->childs_size(), 0);
        } else {
            BOOST_CHECK(false);
        }
    });

    bool before_added_called = false;
    watch.connect_before_added([node1, &before_added_called](auto parent, auto first, auto last) {
        BOOST_CHECK(!before_added_called);
        before_added_called = true;
        BOOST_CHECK(parent == node1);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 1);
        BOOST_CHECK_EQUAL(parent->childs_size(), 0);
    });

    bool after_added_called = false;
    watch.connect_after_added([node1, &after_added_called](auto parent, auto first, auto last) {
        BOOST_CHECK(!after_added_called);
        after_added_called = true;
        BOOST_CHECK(parent == node1);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 1);
        BOOST_CHECK_EQUAL(parent->childs_size(), 2);
    });

    watch.connect_before_removed( [](auto parent, auto first, auto last) {
        BOOST_CHECK(false);
    });

    watch.connect_after_removed([](auto parent, auto first, auto last) {
        BOOST_CHECK(false);
    });

    var_handler(watch_list_impl::tree_info::node{10, L"var", L"my val", L"my type", 2});
    var2_handler(watch_list_impl::tree_info::node{20, L"var2", L"my val 2", L"my type 2", 0});

    BOOST_CHECK(n1_changed_called);
    BOOST_CHECK(n2_changed_called);
    BOOST_CHECK(before_added_called);
    BOOST_CHECK(after_added_called);


    // checking result tree

    BOOST_REQUIRE_EQUAL(watch.root_nodes_size(), 2);
    {
        auto it = watch.root_nodes_begin();
        BOOST_REQUIRE(it != watch.root_nodes_end());
        node1 = *it;

        ++it;
        BOOST_REQUIRE(it != watch.root_nodes_end());
        node2 = *it;
    }

    BOOST_CHECK(node1->name() == L"var");
    BOOST_CHECK(node1->value() == L"my val");
    BOOST_CHECK(node1->type() == L"my type");
    BOOST_CHECK_EQUAL(node1->childs_size(), 2);

    BOOST_CHECK(node2->name() == L"var2");
    BOOST_CHECK(node2->value() == L"my val 2");
    BOOST_CHECK(node2->type() == L"my type 2");
    BOOST_CHECK_EQUAL(node2->childs_size(), 0);


    BOOST_CHECK(impl.verify());
}


/// Tests updating watch tree for watch that was added without implementation
BOOST_AUTO_TEST_CASE(update_tree_watch_add_no_impl) {
    // removing implementation
    watch.set_impl(nullptr);

    // adding watch
    watch.add_watch(L"var");

    // setting implementation. Watch list should call add_watch in implementation for the added watch

    bool set_fmt_opts_called = false;
    MOCK_ADD_CALL(impl, set_fmt_opts, [&set_fmt_opts_called](auto && opts, auto && h) {
        BOOST_CHECK(!set_fmt_opts_called);
        set_fmt_opts_called = true;
    });

    MOCK_ADD_CALL(impl, add_watch, [](const auto & expr, auto && h) {
        BOOST_CHECK(expr == L"var");
        h(watch_list_impl::tree_info::node{10, L"", L"", L"", 0});
    });

    watch.set_impl(&impl);

    BOOST_CHECK(set_fmt_opts_called);

    BOOST_REQUIRE_EQUAL(watch.root_nodes_size(), 1);
    auto node1 = *watch.root_nodes_begin();

    // sending updated watch tree

    watch.connect_root_node_removed([](auto && ...) {
        BOOST_CHECK(false);
    });

    watch.connect_root_node_added([](auto && ...) {
        BOOST_CHECK(false);
    });

    bool changed_called = false;
    watch.connect_node_changed([&changed_called, node1](const watch_list_tree_node * node) {
        BOOST_CHECK(!changed_called);
        changed_called = true;
        BOOST_CHECK(node == node1);

        BOOST_CHECK(node->name() == L"var");
        BOOST_CHECK(node->value() == L"my val");
        BOOST_CHECK(node->type() == L"my type");
        BOOST_CHECK_EQUAL(node->childs_size(), 0);
    });

    watch.connect_before_added([](auto && ...) {
        BOOST_CHECK(false);
    });

    watch.connect_after_added([](auto && ...) {
        BOOST_CHECK(false);
    });

    watch.connect_before_removed( [](auto && ...) {
        BOOST_CHECK(false);
    });

    watch.connect_after_removed([](auto && ...) {
        BOOST_CHECK(false);
    });

    {
        watch_list_impl_tree_info tree;
        tree.add_node({10, L"var", L"my val", L"my type", 0});
        impl.emit_tree_updated_signal(tree);
    }

    BOOST_CHECK(changed_called);


    // checking result tree

    BOOST_REQUIRE_EQUAL(watch.root_nodes_size(), 1);
    {
        auto it = watch.root_nodes_begin();
        BOOST_REQUIRE(it != watch.root_nodes_end());
        node1 = *it;
    }

    BOOST_CHECK(node1->name() == L"var");
    BOOST_CHECK(node1->value() == L"my val");
    BOOST_CHECK(node1->type() == L"my type");
    BOOST_CHECK_EQUAL(node1->childs_size(), 0);

    BOOST_CHECK(impl.verify());
}



BOOST_AUTO_TEST_SUITE_END()


}
