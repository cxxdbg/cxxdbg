// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file watch_list_model_test.cpp
/// Contains unit tests for the watch_list_model class.

#include "test_watch_list_impl.hpp"
#include "cxxdbg/dbg/watch_list.hpp"
#include "cxxdbg/dbg/watch_model.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::test {


struct watch_list_model_test_fixture {
    test_watch_list_impl impl;
    watch_list watch{&impl};
    watch_model model{watch};

    const watch_list_tree_node * make_node(const tree_view_model_row_index & index) {
        return reinterpret_cast<const watch_list_tree_node*>(index.ptr());
    }
};


BOOST_FIXTURE_TEST_SUITE(watch_list_model_test, watch_list_model_test_fixture)


/// Tests empty watch list
BOOST_AUTO_TEST_CASE(test_empty) {
    BOOST_CHECK(model.childs_size() == 0);
}


/// Tests adding single root node to watch list
BOOST_AUTO_TEST_CASE(add_root_single) {
    // listening for root node added signals
    bool signal_emitted = false;

    unsigned int sig_num = 0;

    model.before_added().connect([this, &sig_num](const auto & row, auto first, auto last) {
        BOOST_CHECK(sig_num == 0);

        BOOST_CHECK(!row);
        BOOST_CHECK(first == 0);
        BOOST_CHECK(last == 0);

        ++sig_num;
    });

    model.after_added().connect([this, &sig_num](const auto & row, auto first, auto last) {
        BOOST_CHECK(sig_num == 1);

        BOOST_CHECK(!row);
        BOOST_CHECK(first == 0);
        BOOST_CHECK(last == 0);

        ++sig_num;
    });

    model.before_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.after_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

//    model.after_changed().connect([](const auto & row) {
//        BOOST_CHECK(false);
//    });

    // adding single root node to watch list
    watch_list_impl::tree_info tinfo;
    tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 0));
    impl.emit_tree_updated_signal(tinfo);

    // checking that signals were emitted
    BOOST_CHECK(sig_num == 2);

    // checking that new root node was added
    BOOST_REQUIRE(model.childs_size() == 1);

    auto root = model.child({}, 0);
    BOOST_CHECK(model.text(root, 0) == L"my node");
    BOOST_CHECK(model.text(root, 1) == L"my val");
    BOOST_CHECK(model.text(root, 2) == L"my type");
    BOOST_CHECK(model.childs_size(root) == 0);
    BOOST_CHECK(model.color(root, 0) == 0); // newly added root nodes are not marked
}


/// Tests removing single root node from watch list
BOOST_AUTO_TEST_CASE(remove_root_single) {
    // adding single root node to watch list
    watch_list_impl::tree_info tinfo;
    tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 0));
    impl.emit_tree_updated_signal(tinfo);


    unsigned sig_num = 0;

    model.before_added().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    model.after_added().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    model.before_removed().connect([this, &sig_num](const auto & parent, auto first, auto last) {
        BOOST_CHECK(sig_num == 0);
        ++sig_num;

        BOOST_CHECK(!parent);
        BOOST_CHECK(first == 0);
        BOOST_CHECK(last == 0);
    });

    model.after_removed().connect([this, &sig_num](const auto & parent, auto first, auto last) {
        BOOST_CHECK(sig_num == 1);
        ++sig_num;

        BOOST_CHECK(!parent);
        BOOST_CHECK(first == 0);
        BOOST_CHECK(last == 0);
    });

    model.after_changed().connect([](const auto & index) {
        BOOST_CHECK(false);
    });

    // removing root node
    impl.emit_tree_updated_signal(watch_list_impl::tree_info());

    // checking that signals were emitted
    BOOST_CHECK(sig_num == 2);

    // checking that model is empty
    BOOST_CHECK(model.childs_size() == 0);
}


/// Tests updating single root node
BOOST_AUTO_TEST_CASE(update_root_single) {
    // adding single root node to watch list
    watch_list_impl::tree_info tinfo;
    tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 0));
    impl.emit_tree_updated_signal(tinfo);


    unsigned sig_num = 0, sig_num_target = 0;

    model.before_added().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    model.after_added().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    model.before_removed().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    model.after_removed().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    model.after_changed().connect([this, &sig_num, &sig_num_target](const auto & row) {
        BOOST_CHECK(sig_num == sig_num_target);
        ++sig_num;

        BOOST_CHECK(!model.parent(row));
        BOOST_CHECK(model.index(row) == 0);
    });


    // updating root node
    watch_list_impl::tree_info tinfo2;
    tinfo2.add_node(watch_list_impl::tree_info::node(1, L"my node 2", L"my val 2", L"my type 2", 0));
    impl.emit_tree_updated_signal(tinfo2);

    // checking that signal was emitted
    BOOST_CHECK(sig_num == 1);

    // checking that root node is still in model
    BOOST_REQUIRE(model.childs_size() == 1);
    auto root = model.child({}, 0);
    BOOST_CHECK(model.text(root, 0) == L"my node 2");
    BOOST_CHECK(model.text(root, 1) == L"my val 2");
    BOOST_CHECK(model.text(root, 2) == L"my type 2");
    BOOST_CHECK(model.childs_size(root) == 0);
    BOOST_CHECK(model.color(root, 0) == 1);  // first change considered as update

    // updating root node second time
    sig_num_target = 1;
    watch_list_impl::tree_info tinfo3;
    tinfo3.add_node(watch_list_impl::tree_info::node(1, L"my node 3", L"my val 3", L"my type 3", 0));
    impl.emit_tree_updated_signal(tinfo3);

    BOOST_CHECK(model.text(root, 0) == L"my node 3");
    BOOST_CHECK(model.text(root, 1) == L"my val 3");
    BOOST_CHECK(model.text(root, 2) == L"my type 3");
    BOOST_CHECK(model.childs_size(root) == 0);

    BOOST_CHECK(model.color(root, 0) == 1); // second change considered as update
}


/// Tests single node adding with childs
BOOST_AUTO_TEST_CASE(add_node_signle_childs) {
    unsigned int sig_num = 0;

    model.before_added().connect([this, &sig_num](const auto & parent, auto first, auto last) {
        BOOST_CHECK(sig_num == 0);
        ++sig_num;

        BOOST_CHECK(!parent);
        BOOST_CHECK(first == 0);
        BOOST_CHECK(last == 0);
    });

    model.after_added().connect([this, &sig_num](const auto & parent, auto first, auto last) {
        BOOST_CHECK(sig_num == 1);
        ++sig_num;

        BOOST_CHECK(!parent);
        BOOST_CHECK(first == 0);
        BOOST_CHECK(last == 0);
    });

    model.before_removed().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    model.after_removed().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    model.after_changed().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });


    // adding single root node with 2 childs to watch list
    watch_list_impl::tree_info tinfo;
    tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 2));
    impl.emit_tree_updated_signal(tinfo);


    // checking that signals were emitted
    BOOST_CHECK(sig_num == 2);

    // checking node values
    BOOST_REQUIRE(model.childs_size() == 1);

    auto root = model.child({}, 0);
    BOOST_CHECK(model.text(root, 0) == L"my node");
    BOOST_CHECK(model.text(root, 1) == L"my val");
    BOOST_CHECK(model.text(root, 2) == L"my type");
    BOOST_REQUIRE(model.childs_size(root) == 2);

    BOOST_CHECK(impl.verify());
}


/// Tests childs update after expanding
BOOST_AUTO_TEST_CASE(update_childs_single) {
    // adding single root node with 2 childs to watch list
    watch_list_impl::tree_info tinfo;
    tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 2));
    impl.emit_tree_updated_signal(tinfo);

    bool child1_signal_emitted = false;
    bool child2_signal_emitted = false;
    bool root_signal_emitted = false;

    model.before_added().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    model.after_added().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    model.before_removed().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    model.after_removed().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    model.after_changed().connect(
    [this, &root_signal_emitted, &child1_signal_emitted, &child2_signal_emitted](const auto & row) {
        BOOST_REQUIRE(row);

        if (model.parent(row)) {
            BOOST_CHECK(!model.parent(model.parent(row)));
            BOOST_CHECK(model.index(model.parent(row)) == 0);

            if (model.index(row) == 0) {
                BOOST_CHECK(!child1_signal_emitted);
                child1_signal_emitted = true;
            } else if (model.index(row) == 1) {
                BOOST_CHECK(!child2_signal_emitted);
                child2_signal_emitted = true;
            } else {
                BOOST_CHECK(false);
            }
        } else {
            BOOST_CHECK(!root_signal_emitted);
            root_signal_emitted = true;
            BOOST_CHECK(model.index(row) == 0);
        }
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

    // checking watch model

    // root not changed
    BOOST_REQUIRE(model.childs_size() == 1);
    auto root = model.child({}, 0);
    BOOST_CHECK(model.text(root, 0) == L"my node");
    BOOST_CHECK(model.text(root, 1) == L"my val");
    BOOST_CHECK(model.text(root, 2) == L"my type");

    BOOST_REQUIRE(model.childs_size(root) == 2);

    auto child1 = model.child(root, 0);
    BOOST_CHECK(model.text(child1, 0) == L"child 1");
    BOOST_CHECK(model.text(child1, 1) == L"child 1 val");
    BOOST_CHECK(model.text(child1, 2) == L"child 1 type");
    BOOST_CHECK(model.childs_size(child1) == 0);
    BOOST_CHECK(model.color(child1, 0) == 1);

    auto child2 = model.child(root, 1);
    BOOST_CHECK(model.text(child2, 0) == L"child 2");
    BOOST_CHECK(model.text(child2, 1) == L"child 2 val");
    BOOST_CHECK(model.text(child2, 2) == L"child 2 type");
    BOOST_CHECK(model.childs_size(child2) == 0);
    BOOST_CHECK(model.color(child2, 0) == 1);

    child1_signal_emitted = false;
    child2_signal_emitted = false;

    // updating childs second time
    {
        watch_list_impl::tree_info tinfo2;
        watch_list_impl::tree_info::node root(1, L"my node", L"my val", L"my type", 2);
        root.add_child(watch_list_impl::tree_info::node(2, L"child 1", L"child 1 val", L"child 1 type", 0));
        root.add_child(watch_list_impl::tree_info::node(3, L"child 2", L"child 2 val", L"child 2 type", 0));
        tinfo2.add_node(root);
        impl.emit_tree_updated_signal(tinfo2);
    }

    // checking that signals were emitted
    BOOST_CHECK(child1_signal_emitted);
    BOOST_CHECK(child2_signal_emitted);

    // checking watch model

    // root not changed
    BOOST_REQUIRE(model.childs_size() == 1);
    auto root1 = model.child({}, 0);
    BOOST_CHECK(model.text(root1, 0) == L"my node");
    BOOST_CHECK(model.text(root1, 1) == L"my val");
    BOOST_CHECK(model.text(root1, 2) == L"my type");
    BOOST_CHECK(model.color(root1, 0) == 0);

    BOOST_REQUIRE(model.childs_size(root) == 2);

    auto child11 = model.child(root1, 0);
    BOOST_CHECK(model.text(child11, 0) == L"child 1");
    BOOST_CHECK(model.text(child11, 1) == L"child 1 val");
    BOOST_CHECK(model.text(child11, 2) == L"child 1 type");
    BOOST_CHECK(model.childs_size(child11) == 0);
    BOOST_CHECK(model.color(child11, 0) == 0);

    auto child21 = model.child(root1, 1);
    BOOST_CHECK(model.text(child21, 0) == L"child 2");
    BOOST_CHECK(model.text(child21, 1) == L"child 2 val");
    BOOST_CHECK(model.text(child21, 2) == L"child 2 type");
    BOOST_CHECK(model.childs_size(child21) == 0);
    BOOST_CHECK(model.color(child21, 0) == 0);
}


/// Tests adding/removing child nodes in root node
BOOST_AUTO_TEST_CASE(remove_childs_root_single) { // TODO: maybe correct test name?
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
        auto r = model.child({}, 0);
        auto child1 = model.child(r, 0);
        auto child2 = model.child(r, 1);

        BOOST_CHECK(model.color(child1, 0) == 1);
        BOOST_CHECK(model.color(child2, 1) == 1);
        BOOST_CHECK(model.color(r, 0) == 0); // root not changed
    }

    unsigned int sig_num = 0;

    model.before_added().connect([this, &sig_num](const auto & parent, auto first, auto last) {
        BOOST_CHECK(sig_num == 0);
        ++sig_num;

        BOOST_REQUIRE(parent);
        BOOST_CHECK(!model.parent(parent));
        BOOST_CHECK(model.index(parent) == 0);
        BOOST_CHECK(first == 2);
        BOOST_CHECK(last == 2);
        BOOST_CHECK(model.childs_size(parent) == 2);
    });

    model.after_added().connect([this, &sig_num](const auto & parent, auto first, auto last) {
        BOOST_CHECK(sig_num == 1);
        ++sig_num;

        BOOST_REQUIRE(parent);
        BOOST_CHECK(!model.parent(parent));
        BOOST_CHECK(model.index(parent) == 0);
        BOOST_CHECK(first == 2);
        BOOST_CHECK(last == 2);
        BOOST_CHECK(model.childs_size(parent) == 3);
    });

    model.before_removed().connect([this, &sig_num](auto && ... args) {
        BOOST_CHECK(false);
    });

    model.after_removed().connect([this, &sig_num](auto && ... args) {
        BOOST_CHECK(false);
    });

    model.after_changed().connect([this](const auto & index) {
        auto * node = this->make_node(index);
        BOOST_REQUIRE(node);
        BOOST_CHECK(node->id() == 2 || node->id() == 3 || node->id() == 4);
    });

    // changing number of child nodes
    {
        watch_list_impl::tree_info tinfo;
        watch_list_impl::tree_info::node root = {1, L"my node", L"my val", L"my type", 3};

        // since the logic of updating was changed due to implementing expand/collapse logic
        // we need to assure that nchilds == childs_size or childs_size == 0
        // so we need to populate children
        root.add_child(watch_list_impl::tree_info::node(2, L"child 1", L"child 1 val", L"child 1 type", 0));
        root.add_child(watch_list_impl::tree_info::node(3, L"child 2", L"child 2 val", L"child 2 type", 0));
        root.add_child(watch_list_impl::tree_info::node(4, L"", L"", L"", 0));

        tinfo.add_node(root);
        impl.emit_tree_updated_signal(tinfo);
    }

    // checking that signals were emitted
    BOOST_CHECK(sig_num == 2);

    // checking watch model
    BOOST_REQUIRE(model.childs_size() == 1);

    auto root = model.child({}, 0);
    BOOST_CHECK(model.text(root, 0) == L"my node");
    BOOST_CHECK(model.text(root, 1) == L"my val");
    BOOST_CHECK(model.text(root, 2) == L"my type");
    BOOST_CHECK(model.color(root, 0) == 0);

    // no value/name changes were made yet
    // so red nodes not chilled out yet
    BOOST_REQUIRE(model.childs_size(root) == 3);
    auto child1 = model.child(root, 0);
    BOOST_CHECK(model.text(child1, 0) == L"child 1");
    BOOST_CHECK(model.text(child1, 1) == L"child 1 val");
    BOOST_CHECK(model.text(child1, 2) == L"child 1 type");
    BOOST_CHECK(model.color(child1, 0) == 0);   // already updated

    auto child2 = model.child(root, 1);
    BOOST_CHECK(model.text(child2, 0) == L"child 2");
    BOOST_CHECK(model.text(child2, 1) == L"child 2 val");
    BOOST_CHECK(model.text(child2, 2) == L"child 2 type");
    BOOST_CHECK(model.color(child2, 0) == 0);   // already updated

    auto child3 = model.child(root, 2);
    BOOST_CHECK(model.text(child3, 0) == L"");
    BOOST_CHECK(model.text(child3, 1) == L"");
    BOOST_CHECK(model.text(child3, 2) == L"");
    BOOST_CHECK_EQUAL(model.color(child3, 0), 1);   // newly added should be highlighted


    BOOST_CHECK(impl.verify());
}


/// Tests displaying single root node with hex
BOOST_AUTO_TEST_CASE(single_root_node_hex) {
    // adding single root node to watch list
    watch_list_impl::tree_info tinfo;
    tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 0));
    impl.emit_tree_updated_signal(tinfo);

    MOCK_ADD_CALL(impl, set_fmt_opts, [](auto && opts, auto && handler) {
        watch_list_impl::tree_info tinfo;
        tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my hex val", L"my type", 0));
        handler(tinfo);
    });

    auto opts = watch.fmt_opts();
    opts.set_hex(true);
    watch.set_fmt_opts(opts);

    // checking that new root node was added
    BOOST_REQUIRE(model.childs_size() == 1);

    auto root = model.child({}, 0);
    BOOST_CHECK(model.text(root, 0) == L"my node");
    BOOST_CHECK(model.text(root, 1) == L"my hex val");
    BOOST_CHECK(model.text(root, 2) == L"my type");
    BOOST_CHECK(model.childs_size(root) == 0);
}


/// Tests updating values after setting hex
BOOST_AUTO_TEST_CASE(set_hex_update) {
    // adding single root node to watch list
    watch_list_impl::tree_info tinfo;
    tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my val", L"my type", 0));
    tinfo.add_node(watch_list_impl::tree_info::node(2, L"my node 2", L"my val 2", L"my type", 0));
    impl.emit_tree_updated_signal(tinfo);


    model.before_added().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    model.after_added().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    model.before_removed().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    model.after_removed().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    unsigned int sig_num = 0;
    model.after_changed().connect([this, &sig_num](const auto & row) {
        BOOST_CHECK(sig_num == 0);
        ++sig_num;

        BOOST_REQUIRE(row);
        BOOST_CHECK(!model.parent(row));
        BOOST_CHECK(model.index(row) == 0);
    });

    MOCK_ADD_CALL(impl, set_fmt_opts, [](auto && opts, auto && handler) {
        watch_list_impl::tree_info tinfo;
        tinfo.add_node(watch_list_impl::tree_info::node(1, L"my node", L"my hex val", L"my type", 0));
        tinfo.add_node(watch_list_impl::tree_info::node(2, L"my node 2", L"my val 2", L"my type", 0));
        handler(tinfo);
    });

    auto opts = watch.fmt_opts();
    opts.set_hex(true);
    watch.set_fmt_opts(opts);

    BOOST_CHECK_EQUAL(sig_num, 1);
}


BOOST_AUTO_TEST_SUITE_END()


}
