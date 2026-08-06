// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file watch_list_test.cpp
/// Contains unit tests for watch_list class.

#include "test_watch_list_impl.hpp"
#include "cxxdbg/dbg/watch_list.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::test {


namespace {
    /// Test suite fixture
    struct watch_list_test_fixture {
        test_watch_list_impl impl;
        watch_list list;

        watch_list_test_fixture():
            list(&impl) {}
    };
}


BOOST_FIXTURE_TEST_SUITE(watch_list_test, watch_list_test_fixture)


/// Tests watch list initialization
BOOST_AUTO_TEST_CASE(init) {
    // watch list should be empty after initialization
    BOOST_CHECK(list.root_nodes_size() == 0);
}


/// Tests adding single root node to watch list
BOOST_AUTO_TEST_CASE(add_root_single) {

    // listening for root node added signals
    bool signal_emitted = false;
    list.connect_root_node_added([this, &signal_emitted](const watch_list_tree_node * node) {
        signal_emitted = true;

        BOOST_REQUIRE(node != nullptr);

        // checking that new node is in tree
        BOOST_CHECK(list.root_nodes_size() == 1);
        BOOST_CHECK(list.root_nodes_begin() != list.root_nodes_end());
        BOOST_CHECK(*list.root_nodes_begin() == node);

        // checking new node values
        BOOST_CHECK(node->name() == L"my node");
        BOOST_CHECK(node->value() == L"my val");
        BOOST_CHECK(node->type() == L"my type");
        BOOST_CHECK(node->childs_size() == 0);
        BOOST_CHECK(node->is_marked() == false);
    });

    // listening for root node removed signals
    list.connect_root_node_removed([](const watch_list_tree_node*) {
        BOOST_CHECK(false);
    });

    // listening for node changed signals
//    list.connect_node_changed([](const watch_list_tree_node*) {
//        BOOST_CHECK(false);
//    });

    list.connect_before_added([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_after_added([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_before_removed([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_after_removed([](auto && ... args) {
        BOOST_CHECK(false);
    });

    // adding single root node to watch list
    watch_list_impl::tree_info tinfo;
    tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 0));
    impl.emit_tree_updated_signal(tinfo);

    // checking that signal was emitted
    BOOST_CHECK(signal_emitted == true);

    // checking that new root node was added
    BOOST_CHECK(list.root_nodes_size() == 1);
    BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
    const watch_list_tree_node * node = *list.root_nodes_begin();
    BOOST_REQUIRE(node);
    BOOST_CHECK(node->name() == L"my node");
    BOOST_CHECK(node->value() == L"my val");
    BOOST_CHECK(node->type() == L"my type");
    BOOST_CHECK(node->childs_size() == 0);
}


/// Tests removing single root node from watch list
BOOST_AUTO_TEST_CASE(remove_root_single) {

    // adding single root node to watch list
    watch_list_impl::tree_info tinfo;
    tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 0));
    impl.emit_tree_updated_signal(tinfo);

    // listening for root node removed signals
    bool signal_emitted = false;
    list.connect_root_node_removed([this, &signal_emitted](const watch_list_tree_node * node) {
        signal_emitted = true;

        BOOST_REQUIRE(node != nullptr);

        // checking that root node is still in watch list tree
        BOOST_CHECK(list.root_nodes_size() == 1);
        BOOST_CHECK(list.root_nodes_begin() != list.root_nodes_end());
        BOOST_CHECK(*list.root_nodes_begin() == node);

        // checking new node values
        BOOST_CHECK(node->name() == L"my node");
        BOOST_CHECK(node->value() == L"my val");
        BOOST_CHECK(node->type() == L"my type");
        BOOST_CHECK(node->childs_size() == 0);
    });

    // listening for root node added signals
    list.connect_root_node_added([](const watch_list_tree_node*) {
        BOOST_CHECK(false);
    });

    // listening for node changed signals
    list.connect_node_changed([](const watch_list_tree_node*) {
        BOOST_CHECK(false);
    });

    list.connect_before_added([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_after_added([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_before_removed([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_after_removed([](auto && ... args) {
        BOOST_CHECK(false);
    });


    // removing root node
    impl.emit_tree_updated_signal(watch_list_impl::tree_info());

    // checking that signal was emitted
    BOOST_CHECK(signal_emitted == true);

    // checking that watch list tree is empty
    BOOST_CHECK(list.root_nodes_size() == 0);
}


/// Tests updating single root node
BOOST_AUTO_TEST_CASE(update_root_single) {
    // adding single root node to watch list
    watch_list_impl::tree_info tinfo;
    tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 0));
    impl.emit_tree_updated_signal(tinfo);

    // listening for root node removed signals
    bool sig_num = false;
    list.connect_root_node_removed([](const watch_list_tree_node*) {
        BOOST_CHECK(false);
    });

    // listening for root node added signals
    list.connect_root_node_added([](const watch_list_tree_node*) {
        BOOST_CHECK(false);
    });

    // listening for node changed signals
    list.connect_node_changed([this, &sig_num](const watch_list_tree_node * node) {
        sig_num = true;

        BOOST_REQUIRE(node != nullptr);

        // checking that root node is still in watch list tree
        BOOST_CHECK(list.root_nodes_size() == 1);
        BOOST_CHECK(list.root_nodes_begin() != list.root_nodes_end());
        BOOST_CHECK(*list.root_nodes_begin() == node);

        // checking new node values
        BOOST_CHECK(node->name() == L"my node 2");
        BOOST_CHECK(node->value() == L"my val 2");
        BOOST_CHECK(node->type() == L"my type 2");
        BOOST_CHECK(node->childs_size() == 0);
    });

    list.connect_before_added([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_after_added([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_before_removed([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_after_removed([](auto && ... args) {
        BOOST_CHECK(false);
    });


    // updating root node
    watch_list_impl::tree_info tinfo2;
    tinfo2.add_node(watch_list_impl::tree_info::node(1, L"my node 2", L"my val 2", L"my type 2", 0));
    impl.emit_tree_updated_signal(tinfo2);

    // checking that signal was emitted
    BOOST_CHECK(sig_num == true);

    // checking that root node is still in watch list tree
    BOOST_CHECK(list.root_nodes_size() == 1);
    BOOST_CHECK(list.root_nodes_begin() != list.root_nodes_end());
    const watch_list_tree_node * node = *list.root_nodes_begin();
    BOOST_REQUIRE(node != nullptr);

    // checking new node values
    BOOST_CHECK(node->name() == L"my node 2");
    BOOST_CHECK(node->value() == L"my val 2");
    BOOST_CHECK(node->type() == L"my type 2");
    BOOST_CHECK(node->childs_size() == 0);
}


/// Tests single node adding with childs
BOOST_AUTO_TEST_CASE(add_node_signle_childs) {

    bool sig_num = false;

    // listening for root node removed signals
    list.connect_root_node_removed([](const watch_list_tree_node*) {
        BOOST_CHECK(false);
    });

    // listening for root node added signals
    list.connect_root_node_added([this, &sig_num](const watch_list_tree_node * node) {
        sig_num = true;

        BOOST_REQUIRE(node != nullptr);

        // checking that root node is in watch list tree
        BOOST_CHECK(list.root_nodes_size() == 1);
        BOOST_CHECK(list.root_nodes_begin() != list.root_nodes_end());
        BOOST_CHECK(*list.root_nodes_begin() == node);

        // checking node values
        BOOST_CHECK(node->name() == L"my node");
        BOOST_CHECK(node->value() == L"my val");
        BOOST_CHECK(node->type() == L"my type");

        // checking empty childs nodes

        BOOST_REQUIRE(node->childs_size() == 2);

        const watch_list_tree_node * child1 = node->child_at(0);
        BOOST_REQUIRE(child1 != nullptr);
        BOOST_CHECK(child1->name() == L"");
        BOOST_CHECK(child1->value() == L"");
        BOOST_CHECK(child1->type() == L"");
        BOOST_CHECK(child1->childs_size() == 0);

        const watch_list_tree_node * child2 = node->child_at(1);
        BOOST_REQUIRE(child2 != nullptr);
        BOOST_CHECK(child2->name() == L"");
        BOOST_CHECK(child2->value() == L"");
        BOOST_CHECK(child2->type() == L"");
        BOOST_CHECK(child2->childs_size() == 0);
    });

    // listening for node changed signals
//    list.connect_node_changed([](const watch_list_tree_node*) {
//        BOOST_CHECK(false);
//    });

    list.connect_before_added([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_after_added([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_before_removed([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_after_removed([](auto && ... args) {
        BOOST_CHECK(false);
    });


    // adding single root node with 2 childs to watch list
    watch_list_impl::tree_info tinfo;
    tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 2));
    impl.emit_tree_updated_signal(tinfo);


    // checking that root node added signal was emitted
    BOOST_CHECK(sig_num);

    // checking node values
    BOOST_REQUIRE(list.root_nodes_size() == 1);
    BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
    const watch_list_tree_node * node = *list.root_nodes_begin();
    BOOST_REQUIRE(node != nullptr);

    BOOST_CHECK(node->name() == L"my node");
    BOOST_CHECK(node->value() == L"my val");
    BOOST_CHECK(node->type() == L"my type");

    // checking empty childs nodes

    BOOST_REQUIRE(node->childs_size() == 2);

    const watch_list_tree_node * child1 = node->child_at(0);
    BOOST_REQUIRE(child1 != nullptr);
    BOOST_CHECK(child1->name() == L"");
    BOOST_CHECK(child1->value() == L"");
    BOOST_CHECK(child1->type() == L"");
    BOOST_CHECK(child1->childs_size() == 0);

    const watch_list_tree_node * child2 = node->child_at(1);
    BOOST_REQUIRE(child2 != nullptr);
    BOOST_CHECK(child2->name() == L"");
    BOOST_CHECK(child2->value() == L"");
    BOOST_CHECK(child2->type() == L"");
    BOOST_CHECK(child2->childs_size() == 0);
}


/// Tests childs update after receiving expanded node from implementation
BOOST_AUTO_TEST_CASE(update_childs_single) {

    // adding single root node with 2 childs to watch list
    watch_list_impl::tree_info tinfo;
    tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 2));
    impl.emit_tree_updated_signal(tinfo);

    // getting pointers to childs
    BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
    const watch_list_tree_node * root_node = *list.root_nodes_begin();
    BOOST_REQUIRE(root_node != nullptr);
    BOOST_REQUIRE(root_node->childs_size() == 2);
    const watch_list_tree_node * child1 = root_node->child_at(0);
    const watch_list_tree_node * child2 = root_node->child_at(1);
    BOOST_REQUIRE(child1 != nullptr);
    BOOST_REQUIRE(child2 != nullptr);

    bool child1_signal_emitted = false;
    bool child2_signal_emitted = false;
    bool root_signal_emitted = false;

    // listening for root node removed signals
    list.connect_root_node_removed([](const watch_list_tree_node*) {
        BOOST_CHECK(false);
    });

    // listening for root node added signals
    list.connect_root_node_added([](const watch_list_tree_node * node) {
        BOOST_CHECK(false);
    });

    // listening for node changed signals
    list.connect_node_changed(
    [this, root_node, child1, child2,
     &root_signal_emitted, &child1_signal_emitted, &child2_signal_emitted](const watch_list_tree_node * node) {

        BOOST_REQUIRE(node != nullptr);

        // settings signal emitted falgs and checking node values
        if (node == child1) {
            BOOST_CHECK(!child1_signal_emitted);
            child1_signal_emitted = true;

            BOOST_CHECK(node->name() == L"child 1");
            BOOST_CHECK(node->value() == L"child 1 val");
            BOOST_CHECK(node->type() == L"child 1 type");
            BOOST_CHECK(node->childs_size() == 0);
        } else if (node == child2) {
            BOOST_CHECK(!child2_signal_emitted);
            child2_signal_emitted = true;

            BOOST_CHECK(node->name() == L"child 2");
            BOOST_CHECK(node->value() == L"child 2 val");
            BOOST_CHECK(node->type() == L"child 2 type");
            BOOST_CHECK(node->childs_size() == 0);
        } else if (node == root_node) {
            // changed signal for root node should be called because expand state changed
            BOOST_CHECK(!root_signal_emitted);
            root_signal_emitted = true;

            BOOST_CHECK(node->name() == L"my node");
            BOOST_CHECK(node->value() == L"my val");
            BOOST_CHECK(node->type() == L"my type");
            BOOST_CHECK(node->childs_size() == 2);
        } else {
            BOOST_CHECK(false);
        }
    });

    list.connect_before_added([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_after_added([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_before_removed([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_after_removed([](auto && ... args) {
        BOOST_CHECK(false);
    });


    // updating childs
    {
        watch_list_impl::tree_info tinfo2;
        watch_list_impl::tree_info::node root(1, L"my node", L"my val", L"my type", 2);
        root.add_child(watch_list_impl::tree_info::node(2, L"child 1", L"child 1 val", L"child 1 type", 0));
        root.add_child(watch_list_impl::tree_info::node(3, L"child 2", L"child 2 val", L"child 2 type", 0));
        tinfo2.add_node(root);
        impl.emit_tree_updated_signal(tinfo2);
    }

    // checking that signals were emitted
    BOOST_CHECK(root_signal_emitted);
    BOOST_CHECK(child1_signal_emitted);
    BOOST_CHECK(child2_signal_emitted);

    // checking watch tree

    BOOST_CHECK(list.root_nodes_size() == 1);
    BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
    BOOST_REQUIRE(*list.root_nodes_begin() == root_node);

    BOOST_CHECK(root_node->name() == L"my node");
    BOOST_CHECK(root_node->value() == L"my val");
    BOOST_CHECK(root_node->type() == L"my type");

    BOOST_REQUIRE(root_node->childs_size() == 2);
    BOOST_REQUIRE(root_node->child_at(0) == child1);
    BOOST_REQUIRE(root_node->child_at(1) == child2);

    BOOST_CHECK(child1->name() == L"child 1");
    BOOST_CHECK(child1->value() == L"child 1 val");
    BOOST_CHECK(child1->type() == L"child 1 type");
    BOOST_CHECK(child1->childs_size() == 0);

    BOOST_CHECK(child2->name() == L"child 2");
    BOOST_CHECK(child2->value() == L"child 2 val");
    BOOST_CHECK(child2->type() == L"child 2 type");
    BOOST_CHECK(child2->childs_size() == 0);
}

/// Tests all childs removal with raw data
BOOST_AUTO_TEST_CASE(remove_all_childs_raw_data) {

    // adding single root node with 2 childs to watch list
    watch_list_impl::tree_info tinfo;
    tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 3));
    impl.emit_tree_updated_signal(tinfo);

    // getting pointers to childs
    BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
    const watch_list_tree_node * root_node = *list.root_nodes_begin();
    BOOST_REQUIRE(root_node != nullptr);
    BOOST_REQUIRE(root_node->childs_size() == 3);
    const watch_list_tree_node * child1 = root_node->child_at(0);
    const watch_list_tree_node * child2 = root_node->child_at(1);
    const watch_list_tree_node * child3 = root_node->child_at(2);
    BOOST_REQUIRE(child1 != nullptr);
    BOOST_REQUIRE(child2 != nullptr);
    BOOST_REQUIRE(child3 != nullptr);

    // listening for root node removed signals
    list.connect_root_node_removed([](const watch_list_tree_node*) {
        BOOST_CHECK(false);
    });

    // listening for root node added signals
    list.connect_root_node_added([](const watch_list_tree_node * node) {
        BOOST_CHECK(false);
    });

    // updating childs
    {
        watch_list_impl::tree_info tinfo2;
        watch_list_impl::tree_info::node root(1, L"my node", L"my val", L"my type", 3);
        root.add_child(watch_list_impl::tree_info::node(2, L"child 1", L"child 1 val", L"child 1 type", 0));
        root.add_child(watch_list_impl::tree_info::node(3, L"child 2", L"child 2 val", L"child 2 type", 0));
        root.add_child(watch_list_impl::tree_info::node(4, L"[Raw Data]", L"raw data val", L"raw data type", 0));
        tinfo2.add_node(root);
        impl.emit_tree_updated_signal(tinfo2);
    }

    list.connect_node_changed([this](const watch_list_tree_node * node) {
        BOOST_CHECK(false);
    });

    list.connect_before_added([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_after_added([](auto && ... args) {
        BOOST_CHECK(false);
    });

    std::size_t before_removed_count = 0;

    list.connect_before_removed([this, root_node, &before_removed_count](auto * node, size_t first, size_t last) {
        ++before_removed_count;
        BOOST_CHECK(node == root_node);
        BOOST_CHECK(first == 0);
        BOOST_CHECK(last == 2);
    });

    std::size_t after_removed_count = 0;

    list.connect_after_removed([this, root_node, &after_removed_count](auto * node, size_t first, size_t last) {
        ++after_removed_count;
        BOOST_CHECK(node == root_node);
        BOOST_CHECK(first == 0);
        BOOST_CHECK(last == 2);
    });

    // remove all children
    {
        watch_list_impl::tree_info tinfo2;
        watch_list_impl::tree_info::node root(1, L"my node", L"my val", L"my type", 0);
        tinfo2.add_node(root);
        impl.emit_tree_updated_signal(tinfo2);
    }

    BOOST_CHECK(before_removed_count == 1);
    BOOST_CHECK(after_removed_count == 1);

    // checking watch tree
    BOOST_CHECK(list.root_nodes_size() == 1);
    BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
    BOOST_REQUIRE(*list.root_nodes_begin() == root_node);

    BOOST_CHECK(root_node->name() == L"my node");
    BOOST_CHECK(root_node->value() == L"my val");
    BOOST_CHECK(root_node->type() == L"my type");

    BOOST_REQUIRE(root_node->childs_size() == 0);
}

/// Tests removal inner child when raw data child exists
BOOST_AUTO_TEST_CASE(remove_inner_child_raw_data) {

    // adding single root node with 2 childs to watch list
    watch_list_impl::tree_info tinfo;
    tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 3));
    impl.emit_tree_updated_signal(tinfo);

    // getting pointers to childs
    BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
    const watch_list_tree_node * root_node = *list.root_nodes_begin();
    BOOST_REQUIRE(root_node != nullptr);
    BOOST_REQUIRE(root_node->childs_size() == 3);
    const watch_list_tree_node * child1 = root_node->child_at(0);
    const watch_list_tree_node * child2 = root_node->child_at(1);
    const watch_list_tree_node * child3 = root_node->child_at(2);
    BOOST_REQUIRE(child1 != nullptr);
    BOOST_REQUIRE(child2 != nullptr);
    BOOST_REQUIRE(child3 != nullptr);

    // listening for root node removed signals
    list.connect_root_node_removed([](const watch_list_tree_node*) {
        BOOST_CHECK(false);
    });

    // listening for root node added signals
    list.connect_root_node_added([](const watch_list_tree_node * node) {
        BOOST_CHECK(false);
    });

    // updating childs
    {
        watch_list_impl::tree_info tinfo2;
        watch_list_impl::tree_info::node root(1, L"my node", L"my val", L"my type", 3);
        root.add_child(watch_list_impl::tree_info::node(2, L"child 1", L"child 1 val", L"child 1 type", 0));
        root.add_child(watch_list_impl::tree_info::node(3, L"child 2", L"child 2 val", L"child 2 type", 0));
        root.add_child(watch_list_impl::tree_info::node(4, L"[Raw Data]", L"raw data val", L"raw data type", 0));
        tinfo2.add_node(root);
        impl.emit_tree_updated_signal(tinfo2);
    }

    list.connect_before_added([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_after_added([](auto && ... args) {
        BOOST_CHECK(false);
    });

    std::size_t before_removed_count = 0;

    list.connect_before_removed([this, root_node, &before_removed_count](auto * node, size_t first, size_t last) {
        ++before_removed_count;
        BOOST_CHECK(node == root_node);
        BOOST_CHECK(first == 1);
        BOOST_CHECK(last == 1);
    });

    std::size_t after_removed_count = 0;

    list.connect_after_removed([this, root_node, &after_removed_count](auto * node, size_t first, size_t last) {
        ++after_removed_count;
        BOOST_CHECK(node == root_node);
        BOOST_CHECK(first == 1);
        BOOST_CHECK(last == 1);
    });

    // remove inner child
    {
        watch_list_impl::tree_info tinfo2;
        watch_list_impl::tree_info::node root(1, L"my node", L"my val", L"my type", 2);
        root.add_child(watch_list_impl::tree_info::node(3, L"child 2", L"child 2 val", L"child 2 type", 0));
        root.add_child(watch_list_impl::tree_info::node(4, L"[Raw Data]", L"raw data val", L"raw data type", 0));
        tinfo2.add_node(root);
        impl.emit_tree_updated_signal(tinfo2);
    }

    BOOST_CHECK(before_removed_count == 1);
    BOOST_CHECK(after_removed_count == 1);

    // checking watch tree
    BOOST_CHECK(list.root_nodes_size() == 1);
    BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
    BOOST_REQUIRE(*list.root_nodes_begin() == root_node);

    BOOST_CHECK(root_node->name() == L"my node");
    BOOST_CHECK(root_node->value() == L"my val");
    BOOST_CHECK(root_node->type() == L"my type");

    BOOST_REQUIRE(root_node->childs_size() == 2);
    auto * _child1 = root_node->child_at(0);
    auto * _child2 = root_node->child_at(1);

    BOOST_CHECK(_child1->name() == L"child 2");
    BOOST_CHECK(_child1->value() == L"child 2 val");
    BOOST_CHECK(_child1->type() == L"child 2 type");

    BOOST_CHECK(_child2->name() == L"[Raw Data]");
    BOOST_CHECK(_child2->value() == L"raw data val");
    BOOST_CHECK(_child2->type() == L"raw data type");
}

/// Tests adding inner child when raw data child exists
BOOST_AUTO_TEST_CASE(add_inner_child_raw_data) {

    // adding single root node with 2 childs to watch list
    watch_list_impl::tree_info tinfo;
    tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 3));
    impl.emit_tree_updated_signal(tinfo);

    // getting pointers to childs
    BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
    const watch_list_tree_node * root_node = *list.root_nodes_begin();
    BOOST_REQUIRE(root_node != nullptr);
    BOOST_REQUIRE(root_node->childs_size() == 3);
    const watch_list_tree_node * child1 = root_node->child_at(0);
    const watch_list_tree_node * child2 = root_node->child_at(1);
    const watch_list_tree_node * child3 = root_node->child_at(2);
    BOOST_REQUIRE(child1 != nullptr);
    BOOST_REQUIRE(child2 != nullptr);
    BOOST_REQUIRE(child3 != nullptr);

    // listening for root node removed signals
    list.connect_root_node_removed([](const watch_list_tree_node*) {
        BOOST_CHECK(false);
    });

    // listening for root node added signals
    list.connect_root_node_added([](const watch_list_tree_node * node) {
        BOOST_CHECK(false);
    });

    // updating childs
    {
        watch_list_impl::tree_info tinfo2;
        watch_list_impl::tree_info::node root(1, L"my node", L"my val", L"my type", 3);
        root.add_child(watch_list_impl::tree_info::node(2, L"child 1", L"child 1 val", L"child 1 type", 0));
        root.add_child(watch_list_impl::tree_info::node(3, L"child 2", L"child 2 val", L"child 2 type", 0));
        root.add_child(watch_list_impl::tree_info::node(4, L"[Raw Data]", L"raw data val", L"raw data type", 0));
        tinfo2.add_node(root);
        impl.emit_tree_updated_signal(tinfo2);
    }

    std::size_t before_added_count = 0;

    list.connect_before_added([this, root_node, &before_added_count](auto * node, std::size_t first, std::size_t last) {
        ++before_added_count;
        BOOST_CHECK(node == root_node);
        BOOST_CHECK(first == 2);
        BOOST_CHECK(last == 2);
    });

    std::size_t after_added_count = 0;

    list.connect_after_added([this, root_node, &after_added_count](auto * node, std::size_t first, std::size_t last) {
        ++after_added_count;
        BOOST_CHECK(node == root_node);
        BOOST_CHECK(first == 2);
        BOOST_CHECK(last == 2);
    });

    list.connect_before_removed([](auto && ...) {
        BOOST_CHECK(false);
    });

    std::size_t after_removed_count = 0;

    list.connect_after_removed([](auto && ...) {
        BOOST_CHECK(false);
    });

    // add child
    {
        watch_list_impl::tree_info tinfo2;
        watch_list_impl::tree_info::node root(1, L"my node", L"my val", L"my type", 4);
        root.add_child(watch_list_impl::tree_info::node(2, L"child 1", L"child 1 val", L"child 1 type", 0));
        root.add_child(watch_list_impl::tree_info::node(3, L"child 2", L"child 2 val", L"child 2 type", 0));
        root.add_child(watch_list_impl::tree_info::node(5, L"child 3", L"child 3 val", L"child 3 type", 0));
        root.add_child(watch_list_impl::tree_info::node(4, L"[Raw Data]", L"raw data val", L"raw data type", 0));
        tinfo2.add_node(root);
        impl.emit_tree_updated_signal(tinfo2);
    }

    BOOST_CHECK(before_added_count == 1);
    BOOST_CHECK(after_added_count == 1);

    // checking watch tree
    BOOST_CHECK(list.root_nodes_size() == 1);
    BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
    BOOST_REQUIRE(*list.root_nodes_begin() == root_node);

    BOOST_CHECK(root_node->name() == L"my node");
    BOOST_CHECK(root_node->value() == L"my val");
    BOOST_CHECK(root_node->type() == L"my type");

    BOOST_REQUIRE(root_node->childs_size() == 4);
    auto * _child1 = root_node->child_at(0);
    auto * _child2 = root_node->child_at(1);
    auto * _child3 = root_node->child_at(2);
    auto * _child4 = root_node->child_at(3);

    BOOST_CHECK(_child1->name() == L"child 1");
    BOOST_CHECK(_child1->value() == L"child 1 val");
    BOOST_CHECK(_child1->type() == L"child 1 type");

    BOOST_CHECK(_child2->name() == L"child 2");
    BOOST_CHECK(_child2->value() == L"child 2 val");
    BOOST_CHECK(_child2->type() == L"child 2 type");

    BOOST_CHECK(_child3->name() == L"child 3");
    BOOST_CHECK(_child3->value() == L"child 3 val");
    BOOST_CHECK(_child3->type() == L"child 3 type");

    BOOST_CHECK(_child4->name() == L"[Raw Data]");
    BOOST_CHECK(_child4->value() == L"raw data val");
    BOOST_CHECK(_child4->type() == L"raw data type");
}


/// Tests node and childs update after expanding
BOOST_AUTO_TEST_CASE(test_expand) {

    // adding single root node with 3 childs to watch list
    watch_list_impl::tree_info tinfo;
    tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 3));
    impl.emit_tree_updated_signal(tinfo);

    // getting pointers to childs
    BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
    const watch_list_tree_node * root_node = *list.root_nodes_begin();
    BOOST_REQUIRE(root_node != nullptr);
    BOOST_REQUIRE_EQUAL(root_node->childs_size(), 3);
    const watch_list_tree_node * child1 = root_node->child_at(0);
    const watch_list_tree_node * child2 = root_node->child_at(1);
    const watch_list_tree_node * child3 = root_node->child_at(2);
    BOOST_REQUIRE(child1 != nullptr);
    BOOST_REQUIRE(child2 != nullptr);
    BOOST_REQUIRE(child3 != nullptr);

    int n_child1_changed_emitted = 0;
    int n_child2_changed_emitted = 0;
    int n_child3_changed_emitted = 0;
    int n_child1_childs_before_added_called = 0;
    int n_child2_childs_before_added_called = 0;
    int n_child1_childs_after_added_called = 0;
    int n_child2_childs_after_added_called = 0;

    // listening for root node removed signals
    list.connect_root_node_removed([](const watch_list_tree_node*) {
        BOOST_CHECK(false);
    });

    // listening for root node added signals
    list.connect_root_node_added([](const watch_list_tree_node * node) {
        BOOST_CHECK(false);
    });

    // listening for node changed signals
    list.connect_node_changed(
    [this, child1, child2, child3, root_node, &n_child1_changed_emitted,
    &n_child2_changed_emitted, &n_child3_changed_emitted](const watch_list_tree_node * node) {
        BOOST_REQUIRE(node != nullptr);

        // settings signal emitted falgs and checking node values
        if (node == child1) {
            ++n_child1_changed_emitted;

            BOOST_CHECK(node->name() == L"child 1");
            BOOST_CHECK(node->value() == L"child 1 val");
            BOOST_CHECK(node->type() == L"child 1 type");
            BOOST_CHECK(node->childs_size() == 0);
        } else if (node == child2) {
            ++n_child2_changed_emitted;

            BOOST_CHECK(node->name() == L"child 2");
            BOOST_CHECK(node->value() == L"child 2 val");
            BOOST_CHECK(node->type() == L"child 2 type");
            BOOST_CHECK(node->childs_size() == 0);
        } else if (node == child3) {
            ++n_child3_changed_emitted;

            BOOST_CHECK(node->name() == L"child 3");
            BOOST_CHECK(node->value() == L"child 3 val");
            BOOST_CHECK(node->type() == L"child 3 type");
            BOOST_CHECK(node->childs_size() == 0);
        } else {
            BOOST_CHECK(false);
        }
    });

    list.connect_before_added(
    [&n_child1_childs_before_added_called, &n_child2_childs_before_added_called,
    child1, child2](auto * node, auto first, auto last) {
        if (node == child1) {
            ++n_child1_childs_before_added_called;

            BOOST_CHECK_EQUAL(first, 0);
            BOOST_CHECK_EQUAL(last, 0);
            BOOST_CHECK_EQUAL(node->childs_size(), 0);
        } else if (node == child2) {
            ++n_child2_childs_before_added_called;

            BOOST_CHECK_EQUAL(first, 0);
            BOOST_CHECK_EQUAL(last, 2);
            BOOST_CHECK_EQUAL(node->childs_size(), 0);
        } else {
            BOOST_CHECK(false);
        }
    });

    list.connect_after_added(
    [&n_child1_childs_after_added_called, &n_child2_childs_after_added_called,
    child1, child2](auto * node, auto first, auto last) {
        if (node == child1) {
            ++n_child1_childs_after_added_called;

            BOOST_CHECK_EQUAL(first, 0);
            BOOST_CHECK_EQUAL(last, 0);
            BOOST_CHECK_EQUAL(node->childs_size(), 1);
        } else if (node == child2) {
            ++n_child2_childs_after_added_called;

            BOOST_CHECK_EQUAL(first, 0);
            BOOST_CHECK_EQUAL(last, 2);
            BOOST_CHECK_EQUAL(node->childs_size(), 3);
        } else {
            BOOST_CHECK(false);
        }
    });

    list.connect_before_removed([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_after_removed([](auto && ... args) {
        BOOST_CHECK(false);
    });


    // expanding node
    MOCK_ADD_CALL(impl, expand_node, [](auto id, auto && handl) {
        watch_list_impl::tree_info::node root(1, L"my node", L"my val", L"my type", 3);
        root.add_child(watch_list_impl::tree_info::node(2, L"child 1", L"child 1 val", L"child 1 type", 1));
        root.add_child(watch_list_impl::tree_info::node(3, L"child 2", L"child 2 val", L"child 2 type", 3));
        root.add_child(watch_list_impl::tree_info::node(4, L"child 3", L"child 3 val", L"child 3 type", 0));
        handl(root);
    });

    list.expand_node(root_node);


    // checking that signals were emitted
    BOOST_CHECK_EQUAL(n_child1_changed_emitted, 1);
    BOOST_CHECK_EQUAL(n_child2_changed_emitted, 1);
    BOOST_CHECK_EQUAL(n_child3_changed_emitted, 1);
    BOOST_CHECK_EQUAL(n_child1_childs_before_added_called, 1);
    BOOST_CHECK_EQUAL(n_child1_childs_after_added_called, 1);
    BOOST_CHECK_EQUAL(n_child2_childs_before_added_called, 1);
    BOOST_CHECK_EQUAL(n_child2_childs_after_added_called, 1);

    // checking watch tree

    BOOST_CHECK(list.root_nodes_size() == 1);
    BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
    BOOST_REQUIRE(*list.root_nodes_begin() == root_node);

    BOOST_CHECK(root_node->name() == L"my node");
    BOOST_CHECK(root_node->value() == L"my val");
    BOOST_CHECK(root_node->type() == L"my type");

    BOOST_REQUIRE_EQUAL(root_node->childs_size(), 3);
    BOOST_REQUIRE(root_node->child_at(0) == child1);
    BOOST_REQUIRE(root_node->child_at(1) == child2);
    BOOST_REQUIRE(root_node->child_at(2) == child3);

    BOOST_CHECK(child1->name() == L"child 1");
    BOOST_CHECK(child1->value() == L"child 1 val");
    BOOST_CHECK(child1->type() == L"child 1 type");
    BOOST_CHECK_EQUAL(child1->childs_size(), 1);

    BOOST_CHECK(child2->name() == L"child 2");
    BOOST_CHECK(child2->value() == L"child 2 val");
    BOOST_CHECK(child2->type() == L"child 2 type");
    BOOST_CHECK_EQUAL(child2->childs_size(), 3);

    BOOST_CHECK(child3->name() == L"child 3");
    BOOST_CHECK(child3->value() == L"child 3 val");
    BOOST_CHECK(child3->type() == L"child 3 type");
    BOOST_CHECK_EQUAL(child3->childs_size(), 0);
}


/// Tests node collapsing
BOOST_AUTO_TEST_CASE(test_collapse) {
    // adding single root node with 3 childs to watch list
    watch_list_impl::tree_info tinfo;
    tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 3));
    impl.emit_tree_updated_signal(tinfo);

    // getting pointers to childs
    BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
    const watch_list_tree_node * root_node = *list.root_nodes_begin();
    BOOST_REQUIRE(root_node != nullptr);
    BOOST_REQUIRE_EQUAL(root_node->childs_size(), 3);
    const watch_list_tree_node * child1 = root_node->child_at(0);
    const watch_list_tree_node * child2 = root_node->child_at(1);
    const watch_list_tree_node * child3 = root_node->child_at(2);
    BOOST_REQUIRE(child1 != nullptr);
    BOOST_REQUIRE(child2 != nullptr);
    BOOST_REQUIRE(child3 != nullptr);

    // expanding root node
    MOCK_ADD_CALL(impl, expand_node, [](auto id, auto && handl) {
        watch_list_impl::tree_info::node root(1, L"my node", L"my val", L"my type", 3);
        root.add_child(watch_list_impl::tree_info::node(2, L"child 1", L"child 1 val", L"child 1 type", 1));
        root.add_child(watch_list_impl::tree_info::node(3, L"child 2", L"child 2 val", L"child 2 type", 3));
        root.add_child(watch_list_impl::tree_info::node(4, L"child 3", L"child 3 val", L"child 3 type", 0));
        handl(root);
    });
    list.expand_node(root_node);

    int n_child1_changed_emitted = 0;
    int n_child2_changed_emitted = 0;
    int n_child3_changed_emitted = 0;
    int n_child1_childs_before_removed_called = 0;
    int n_child2_childs_before_removed_called = 0;
    int n_child1_childs_after_removed_called = 0;
    int n_child2_childs_after_removed_called = 0;

    // listening for root node removed signals
    list.connect_root_node_removed([](const watch_list_tree_node*) {
        BOOST_CHECK(false);
    });

    // listening for root node added signals
    list.connect_root_node_added([](const watch_list_tree_node * node) {
        BOOST_CHECK(false);
    });

    // listening for node changed signals
    list.connect_node_changed(
    [this, child1, child2, child3, root_node, &n_child1_changed_emitted,
    &n_child2_changed_emitted, &n_child3_changed_emitted](const watch_list_tree_node * node) {
        BOOST_REQUIRE(node != nullptr);

        // settings signal emitted falgs and checking node values
        if (node == child1) {
            ++n_child1_changed_emitted;

            BOOST_CHECK_EQUAL(node->id(), watch_list::undef_node_id);
            BOOST_CHECK(node->name() == L"");
            BOOST_CHECK(node->value() == L"");
            BOOST_CHECK(node->type() == L"");
            BOOST_CHECK(node->childs_size() == 1);
        } else if (node == child2) {
            ++n_child2_changed_emitted;

            BOOST_CHECK_EQUAL(node->id(), watch_list::undef_node_id);
            BOOST_CHECK(node->name() == L"");
            BOOST_CHECK(node->value() == L"");
            BOOST_CHECK(node->type() == L"");
            BOOST_CHECK(node->childs_size() == 3);
        } else if (node == child3) {
            ++n_child3_changed_emitted;

            BOOST_CHECK_EQUAL(node->id(), watch_list::undef_node_id);
            BOOST_CHECK(node->name() == L"");
            BOOST_CHECK(node->value() == L"");
            BOOST_CHECK(node->type() == L"");
            BOOST_CHECK(node->childs_size() == 0);
        } else {
            BOOST_CHECK(false);
        }
    });

    list.connect_before_added([](auto && ...) {
        BOOST_CHECK(false);
    });

    list.connect_after_added([](auto && ...) {
        BOOST_CHECK(false);
    });

    list.connect_before_removed(
    [&n_child1_childs_before_removed_called, &n_child2_childs_before_removed_called,
    child1, child2](auto * node, auto first, auto last) {
        if (node == child1) {
            ++n_child1_childs_before_removed_called;

            BOOST_CHECK_EQUAL(first, 0);
            BOOST_CHECK_EQUAL(last, 0);
            BOOST_CHECK_EQUAL(node->childs_size(), 1);
        } else if (node == child2) {
            ++n_child2_childs_before_removed_called;

            BOOST_CHECK_EQUAL(first, 0);
            BOOST_CHECK_EQUAL(last, 2);
            BOOST_CHECK_EQUAL(node->childs_size(), 3);
        } else {
            BOOST_CHECK(false);
        }
    });

    list.connect_after_removed(
    [&n_child1_childs_after_removed_called, &n_child2_childs_after_removed_called,
    child1, child2](auto * node, auto first, auto last) {
        if (node == child1) {
            ++n_child1_childs_after_removed_called;

            BOOST_CHECK_EQUAL(first, 0);
            BOOST_CHECK_EQUAL(last, 0);
            BOOST_CHECK_EQUAL(node->childs_size(), 0);
        } else if (node == child2) {
            ++n_child2_childs_after_removed_called;

            BOOST_CHECK_EQUAL(first, 0);
            BOOST_CHECK_EQUAL(last, 2);
            BOOST_CHECK_EQUAL(node->childs_size(), 0);
        } else {
            BOOST_CHECK(false);
        }
    });


    // collapsing node
    MOCK_ADD_CALL(impl, collapse_node, [](auto id, auto && handl) {
        watch_list_impl::tree_info::node root(1, L"my node", L"my val", L"my type", 3);
        handl(root);
    });
    list.collapse_node(root_node);


    // checking that signals were emitted
    BOOST_CHECK_EQUAL(n_child1_changed_emitted, 1);
    BOOST_CHECK_EQUAL(n_child2_changed_emitted, 1);
    BOOST_CHECK_EQUAL(n_child3_changed_emitted, 1);
    BOOST_CHECK_EQUAL(n_child1_childs_before_removed_called, 1);
    BOOST_CHECK_EQUAL(n_child1_childs_after_removed_called, 1);
    BOOST_CHECK_EQUAL(n_child2_childs_before_removed_called, 1);
    BOOST_CHECK_EQUAL(n_child2_childs_after_removed_called, 1);


    // checking watch tree

    BOOST_CHECK(list.root_nodes_size() == 1);
    BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
    BOOST_REQUIRE(*list.root_nodes_begin() == root_node);

    BOOST_CHECK(root_node->name() == L"my node");
    BOOST_CHECK(root_node->value() == L"my val");
    BOOST_CHECK(root_node->type() == L"my type");

    BOOST_REQUIRE_EQUAL(root_node->childs_size(), 3);
    BOOST_REQUIRE(root_node->child_at(0) == child1);
    BOOST_REQUIRE(root_node->child_at(1) == child2);
    BOOST_REQUIRE(root_node->child_at(2) == child3);

    BOOST_CHECK(child1->id() == watch_list::undef_node_id);
    BOOST_CHECK(child1->name() == L"");
    BOOST_CHECK(child1->value() == L"");
    BOOST_CHECK(child1->type() == L"");
    BOOST_CHECK(child1->expand_state() == tree_view_expand_state::collapsed);
    BOOST_CHECK_EQUAL(child1->childs_size(), 0);

    BOOST_CHECK(child2->id() == watch_list::undef_node_id);
    BOOST_CHECK(child2->name() == L"");
    BOOST_CHECK(child2->value() == L"");
    BOOST_CHECK(child2->type() == L"");
    BOOST_CHECK(child2->expand_state() == tree_view_expand_state::collapsed);
    BOOST_CHECK_EQUAL(child2->childs_size(), 0);

    BOOST_CHECK(child3->id() == watch_list::undef_node_id);
    BOOST_CHECK(child3->name() == L"");
    BOOST_CHECK(child3->value() == L"");
    BOOST_CHECK(child3->type() == L"");
    BOOST_CHECK(child3->expand_state() == tree_view_expand_state::collapsed);
    BOOST_CHECK_EQUAL(child3->childs_size(), 0);
}


/// Tests removing child nodes in root node
BOOST_AUTO_TEST_CASE(remove_childs_root_single) {

    // adding single root node with 2 childs to watch list
    {
        watch_list_impl::tree_info tinfo;
        tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 2));
        impl.emit_tree_updated_signal(tinfo);
    }

    // expanding childs
    {
        watch_list_impl::tree_info tinfo2;
        watch_list_impl::tree_info::node root(1, L"my node", L"my val", L"my type", 2);
        root.add_child(watch_list_impl::tree_info::node(2, L"child 1", L"child 1 val", L"child 1 type", 0));
        root.add_child(watch_list_impl::tree_info::node(3, L"child 2", L"child 2 val", L"child 2 type", 0));
        tinfo2.add_node(root);
        impl.emit_tree_updated_signal(tinfo2);
    }

    // getting root node
    BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
    const watch_list_tree_node * root_node = *list.root_nodes_begin();
    BOOST_REQUIRE(root_node != nullptr);

    unsigned int sig_num = 0;

    // listening for root node removed signals
    list.connect_root_node_removed([](const watch_list_tree_node*) {
        BOOST_CHECK(false);
    });

    // listening for root node added signals
    list.connect_root_node_added([](const watch_list_tree_node *) {
        BOOST_CHECK(false);
    });

    // listening for node changed signals
    list.connect_node_changed([](const watch_list_tree_node * node) {
        // now  child 1 becomes child 2
#if false
        std::cout << "---------------- node changed begin ----------------" << std::endl;
#endif
        BOOST_CHECK(node->id() == 3);
#if false
        std::wcout << L"node changed name = " << node->name() << std::endl;
        std::cout << "---------------- node changed end ----------------" << std::endl;
#endif
    });

    list.connect_before_added([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_after_added([](auto && ... args) {
        BOOST_CHECK(false);
    });

    // listening for before node childs changed signal
    list.connect_before_removed([&sig_num, root_node](auto * node, size_t first, size_t last) {
#if false
        std::cout << "---------------- before removed, first = " << first << " last = " << last << " ----------------" << std::endl;
#endif
        if (sig_num > 0) {
            assert(false);
        }
        BOOST_CHECK(sig_num == 0);
        ++sig_num;

        // checking that node is root node
        BOOST_REQUIRE(node == root_node);

        // checking node values
        BOOST_CHECK(node->name() == L"my node");
        BOOST_CHECK(node->value() == L"my val");
        BOOST_CHECK(node->type() == L"my type");
        BOOST_REQUIRE(node->childs_size() == 2);

        const watch_list_tree_node * child = node->child_at(0);
        BOOST_REQUIRE(child != nullptr);
        BOOST_CHECK(child->name() == L"child 1");
        BOOST_CHECK(child->value() == L"child 1 val");
        BOOST_CHECK(child->type() == L"child 1 type");
        BOOST_CHECK(child->childs_size() == 0);

        const watch_list_tree_node * child2 = node->child_at(1);
        BOOST_REQUIRE(child2 != nullptr);
        BOOST_CHECK(child2->name() == L"child 2");
        BOOST_CHECK(child2->value() == L"child 2 val");
        BOOST_CHECK(child2->type() == L"child 2 type");
        BOOST_CHECK(child2->childs_size() == 0);

        BOOST_CHECK(first == 1);
        BOOST_CHECK(last == 1);
#if false
        std::cout << "---------------- before removed end ----------------" << std::endl;
#endif
    });

    // listening for after node childs changed signal
    list.connect_after_removed([&sig_num, root_node](watch_list_tree_node * node, size_t first, size_t last) {
#if false
        std::cout << "---------------- after removed, first = " << first << " last = " << last << " ----------------" << std::endl;
#endif
        BOOST_CHECK(sig_num == 1);
        ++sig_num;

        // checking that node is root node
        BOOST_REQUIRE(node == root_node);

        // checking node values
        BOOST_CHECK(node->name() == L"my node");
        BOOST_CHECK(node->value() == L"my val");
        BOOST_CHECK(node->type() == L"my type");
        BOOST_REQUIRE(node->childs_size() == 1);

        // children are removed from the end
        // so child 2 is removed and then child 1 becomes child 2
        // but now it is still child 1
        const watch_list_tree_node * child = node->child_at(0);
        BOOST_REQUIRE(child != nullptr);
        BOOST_CHECK(child->name() == L"child 1");
        BOOST_CHECK(child->value() == L"child 1 val");
        BOOST_CHECK(child->type() == L"child 1 type");
        BOOST_CHECK(child->childs_size() == 0);

        BOOST_CHECK(first == 1);
        BOOST_CHECK(last == 1);

#if false
        std::cout << "---------------- after removed end ----------------" << std::endl;
#endif
    });

    // changing number of child nodes
    {
        watch_list_impl::tree_info tinfo;
        watch_list_impl::tree_info::node root{1, L"my node", L"my val", L"my type", 1};
        root.add_child({3, L"child 2", L"child 2 val", L"child 2 type", 0});
        tinfo.add_node(root);
        impl.emit_tree_updated_signal(tinfo);
    }

    // checking that signal was emitted
    BOOST_CHECK(sig_num == 2);

    // checking watch tree
    BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
    BOOST_REQUIRE(*list.root_nodes_begin() == root_node);
    BOOST_CHECK(root_node->name() == L"my node");
    BOOST_CHECK(root_node->value() == L"my val");
    BOOST_CHECK(root_node->type() == L"my type");
    BOOST_REQUIRE(root_node->childs_size() == 1);

    const watch_list_tree_node * child = root_node->child_at(0);
    BOOST_REQUIRE(child != nullptr);
    BOOST_CHECK(child->name() == L"child 2");
    BOOST_CHECK(child->value() == L"child 2 val");
    BOOST_CHECK(child->type() == L"child 2 type");
    BOOST_CHECK(child->childs_size() == 0);
}


// /// Tests expanding already expanded root node in watch list
// BOOST_AUTO_TEST_CASE(expand_expanded_root_node) {
//     // adding single root node with 2 childs to watch list
//     {
//         watch_list_impl::tree_info tinfo;
//         watch_list_impl::tree_info::node n{13, L"my node", L"my val", L"my hex", L"my type", 2};
//         tinfo.add_node(n);
//         impl.emit_tree_updated_signal(tinfo);
//     }
//     {
//         watch_list_impl::tree_info tinfo;
//         watch_list_impl::tree_info::node n{13, L"my node", L"my val", L"my hex", L"my type", 2};
//         n.add_child({15, L"my child 1", L"my val 1", L"my hex", L"my type", 0});
//         n.add_child({16, L"my child 2", L"my val 2", L"my hex", L"my type", 0});
//         tinfo.add_node(n);
//         impl.emit_tree_updated_signal(tinfo);
//     }

//     // getting root node
//     BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
//     const watch_list_tree_node * root_node = *list.root_nodes_begin();
//     BOOST_REQUIRE(root_node != nullptr);

//     // expanding root node
//     list.expand_node(root_node);
// }


// /// Tests expanding root node twice
// BOOST_AUTO_TEST_CASE(expand_root_node_twice) {
//     // adding single root node with 2 childs to watch list
//     {
//         watch_list_impl::tree_info tinfo;
//         tinfo.add_node(watch_list_impl::tree_info::node(13, L"my node", L"my val", L"my hex", L"my type", 2));
//         impl.emit_tree_updated_signal(tinfo);
//     }

//     MOCK_ADD_CALL(impl, expand_node, [](auto id, auto && handl) {
//         BOOST_CHECK(id == 13);
//     });

//     // getting root node
//     BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
//     const watch_list_tree_node * root_node = *list.root_nodes_begin();
//     BOOST_REQUIRE(root_node != nullptr);

//     // expanding root node
//     list.expand_node(root_node);
//     list.expand_node(root_node);

//     BOOST_CHECK(impl.verify());
// }


/// Tests setting pointer to new implementation
BOOST_AUTO_TEST_CASE(set_impl) {
    // adding root nodes to watch list
    {
        watch_list_impl::tree_info tinfo;
        tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 2));
        tinfo.add_node(watch_list_impl::tree_info::node(2, L"my node 2", L"my val 2", L"my type 2", 0));
        impl.emit_tree_updated_signal(tinfo);
    }

    auto it = list.root_nodes_begin();

    BOOST_REQUIRE(it != list.root_nodes_end());
    auto root1 = *it;

    ++it;
    BOOST_REQUIRE(it != list.root_nodes_end());
    auto root2 = *it;

    // resetting pointer to new implementation. Watch list should send removed signal for all root nodes

    int n_removed_called = 0;
    list.connect_root_node_removed([&n_removed_called, root1, root2](const watch_list_tree_node * node) {
        if (n_removed_called == 0) {
            BOOST_CHECK(node == root2);
        } else if (n_removed_called == 1) {
            BOOST_CHECK(node == root1);
        } else {
            BOOST_CHECK(false);
        }

        ++n_removed_called;
    });

    list.connect_root_node_added([](auto && ...) {
        BOOST_CHECK(false);
    });

    list.connect_node_changed([](auto && ...) {
        BOOST_CHECK(false);
    });

    list.connect_before_added([](auto && ...) {
        BOOST_CHECK(false);
    });

    list.connect_after_added([](auto && ...) {
        BOOST_CHECK(false);
    });

    list.connect_before_removed( [](auto && ...) {
        BOOST_CHECK(false);
    });

    list.connect_after_removed([](auto && ...) {
        BOOST_CHECK(false);
    });


    list.set_impl(nullptr);

    BOOST_CHECK_EQUAL(n_removed_called, 2);
    BOOST_CHECK_EQUAL(list.root_nodes_size(), 0);
    BOOST_CHECK(impl.verify());
}


/// Tests updating tree after setting new format options
BOOST_AUTO_TEST_CASE(set_fmt_opts) {

    // adding single root node with 2 childs to watch list
    {
        watch_list_impl::tree_info tinfo2;
        watch_list_impl::tree_info::node root(1, L"my node", L"my val", L"my type", 2);
        root.add_child(watch_list_impl::tree_info::node(2, L"child 1", L"child 1 val", L"child 1 type", 0));
        root.add_child(watch_list_impl::tree_info::node(3, L"child 2", L"child 2 val", L"child 2 type", 0));
        tinfo2.add_node(root);
        impl.emit_tree_updated_signal(tinfo2);
    }

    // changing child 2 to make it marked
    {
        watch_list_impl::tree_info tinfo2;
        watch_list_impl::tree_info::node root(1, L"my node", L"my val", L"my type", 2);
        root.add_child(watch_list_impl::tree_info::node(2, L"child 1", L"child 1 val", L"child 1 type", 0));
        root.add_child(watch_list_impl::tree_info::node(3, L"child 2", L"child 2 new val", L"child 2 type", 0));
        tinfo2.add_node(root);
        impl.emit_tree_updated_signal(tinfo2);
    }

    // getting pointers to childs
    BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
    const watch_list_tree_node * root_node = *list.root_nodes_begin();
    BOOST_REQUIRE(root_node != nullptr);
    BOOST_REQUIRE(root_node->childs_size() == 2);
    const watch_list_tree_node * child1 = root_node->child_at(0);
    const watch_list_tree_node * child2 = root_node->child_at(1);
    BOOST_REQUIRE(child1 != nullptr);
    BOOST_REQUIRE(child2 != nullptr);

    bool child1_signal_emitted = false;
    bool child3_signal_emitted = false;
    bool before_added_emitted = false;
    bool after_added_emitted = false;

    // listening for root node removed signals
    list.connect_root_node_removed([](const watch_list_tree_node*) {
        BOOST_CHECK(false);
    });

    // listening for root node added signals
    list.connect_root_node_added([](const watch_list_tree_node * node) {
        BOOST_CHECK(false);
    });

    // listening for node changed signals
    list.connect_node_changed(
    [this, root_node, child1, child2,
     &child1_signal_emitted,
     &child3_signal_emitted](const watch_list_tree_node * node) {

        BOOST_REQUIRE(node != nullptr);

        // settings signal emitted falgs and checking node values
        if (node == child1) {
            BOOST_CHECK(!child1_signal_emitted);
            child1_signal_emitted = true;

            BOOST_CHECK(node->name() == L"child 1");
            BOOST_CHECK(node->value() == L"child 1 new val");
            BOOST_CHECK(node->type() == L"child 1 type");
            BOOST_CHECK(node->childs_size() == 0);
        } else {
            BOOST_CHECK(!child3_signal_emitted);
            child3_signal_emitted = true;

            BOOST_CHECK(node->name() == L"child 3");
            BOOST_CHECK(node->value() == L"child 3 val");
            BOOST_CHECK(node->type() == L"child 3 type");
            BOOST_CHECK(node->childs_size() == 0);
        }
    });

    list.connect_before_added([&before_added_emitted, root_node](auto parent, auto first, auto last) {
        BOOST_CHECK(!before_added_emitted);
        before_added_emitted = true;

        BOOST_CHECK(parent == root_node);
        BOOST_CHECK_EQUAL(first, 2);
        BOOST_CHECK_EQUAL(last, 2);
        BOOST_CHECK_EQUAL(root_node->childs_size(), 2);
    });

    list.connect_after_added([&after_added_emitted, root_node](auto parent, auto first, auto last) {
        BOOST_CHECK(!after_added_emitted);
        after_added_emitted = true;

        BOOST_CHECK(parent == root_node);
        BOOST_CHECK_EQUAL(first, 2);
        BOOST_CHECK_EQUAL(last, 2);
        BOOST_CHECK_EQUAL(root_node->childs_size(), 3);
    });

    list.connect_before_removed([](auto && ... args) {
        BOOST_CHECK(false);
    });

    list.connect_after_removed([](auto && ... args) {
        BOOST_CHECK(false);
    });

    MOCK_ADD_CALL(impl, set_fmt_opts, [](auto && opts, auto && handl) {
        watch_list_impl::tree_info tinfo2;
        watch_list_impl::tree_info::node root(1, L"my node", L"my val", L"my type", 3);
        root.add_child(watch_list_impl::tree_info::node(2, L"child 1", L"child 1 new val", L"child 1 type", 0));
        root.add_child(watch_list_impl::tree_info::node(3, L"child 2", L"child 2 new val", L"child 2 type", 0));
        root.add_child(watch_list_impl::tree_info::node(4, L"child 3", L"child 3 val", L"child 3 type", 0));
        tinfo2.add_node(root);
        handl(tinfo2);
    });

    // setting format options
    list.set_fmt_opts({});

    // checking that signals were emitted
    BOOST_CHECK(child1_signal_emitted);
    BOOST_CHECK(child3_signal_emitted);
    BOOST_CHECK(before_added_emitted);
    BOOST_CHECK(after_added_emitted);

    // checking watch tree

    BOOST_CHECK(list.root_nodes_size() == 1);
    BOOST_REQUIRE(list.root_nodes_begin() != list.root_nodes_end());
    BOOST_REQUIRE(*list.root_nodes_begin() == root_node);
    BOOST_REQUIRE_EQUAL(root_node->childs_size(), 3);

    BOOST_CHECK(root_node->name() == L"my node");
    BOOST_CHECK(root_node->value() == L"my val");
    BOOST_CHECK(root_node->type() == L"my type");
    BOOST_CHECK(!root_node->is_marked());

    BOOST_REQUIRE(root_node->childs_size() == 3);
    BOOST_REQUIRE(root_node->child_at(0) == child1);
    BOOST_REQUIRE(root_node->child_at(1) == child2);

    BOOST_CHECK(child1->name() == L"child 1");
    BOOST_CHECK(child1->value() == L"child 1 new val");
    BOOST_CHECK(child1->type() == L"child 1 type");
    BOOST_CHECK(child1->childs_size() == 0);
    BOOST_CHECK(!child1->is_marked());

    BOOST_CHECK(child2->name() == L"child 2");
    BOOST_CHECK(child2->value() == L"child 2 new val");
    BOOST_CHECK(child2->type() == L"child 2 type");
    BOOST_CHECK(child2->childs_size() == 0);
    // marked flag should be preserved for child 2
    BOOST_CHECK(child2->is_marked());

    auto child3 = root_node->child_at(2);
    BOOST_CHECK(child3->name() == L"child 3");
    BOOST_CHECK(child3->value() == L"child 3 val");
    BOOST_CHECK(child3->type() == L"child 3 type");
    BOOST_CHECK(child3->childs_size() == 0);
    BOOST_CHECK(!child3->is_marked());

    BOOST_CHECK(impl.verify());
}


BOOST_AUTO_TEST_SUITE_END()


}
