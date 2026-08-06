// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file simple_tree_view_model_test.cpp
/// Contains unit tests for the simple_tree_view_model class.

#include "../simple_tree_view_model.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::test {


BOOST_AUTO_TEST_SUITE(simple_tree_view_model_test)


/// Tests initialization of tree view model
BOOST_AUTO_TEST_CASE(init) {
    simple_tree_view_model mdl{2, true};
    BOOST_CHECK_EQUAL(mdl.columns_size(), 2);
    BOOST_CHECK(mdl.column_name(0) == L"");
    BOOST_CHECK(mdl.column_name(1) == L"");
    BOOST_CHECK(mdl.childs_size({}) == 0);
}


/// Tests setting column names
BOOST_AUTO_TEST_CASE(set_column_name) {
    simple_tree_view_model mdl{2, true};
    mdl.set_column_name(0, L"aaa");
    mdl.set_column_name(1, L"column 2");

    BOOST_CHECK(mdl.column_name(0) == L"aaa");
    BOOST_CHECK(mdl.column_name(1) == L"column 2");
}


/// Tests adding root nodes
BOOST_AUTO_TEST_CASE(add_root_node) {
    simple_tree_view_model mdl{2, true};

    int before_added_signum = 0;
    mdl.before_added().connect([&](auto row, size_t start, size_t end) {
        BOOST_CHECK_EQUAL(before_added_signum, 0);
        BOOST_CHECK(!row.is_valid());
        BOOST_CHECK_EQUAL(start, 0);
        BOOST_CHECK_EQUAL(end, 1);
        BOOST_CHECK_EQUAL(mdl.childs_size({}), 0);

        ++before_added_signum;
    });

    int after_added_signum = 0;
    mdl.after_added().connect([&](auto row, size_t start, size_t end) {
        BOOST_CHECK_EQUAL(after_added_signum, 0);
        BOOST_CHECK(!row.is_valid());
        BOOST_CHECK_EQUAL(start, 0);
        BOOST_CHECK_EQUAL(end, 1);
        BOOST_CHECK_EQUAL(mdl.childs_size({}), 2);

        ++after_added_signum;
    });

    mdl.before_removed().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });

    mdl.after_removed().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });

    mdl.after_changed().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });

    mdl.before_layout_changed().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });

    mdl.after_layout_changed().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });


    mdl.insert({}, 0, {{L"aaaa", L"bbb"}, {L"xxx", L"yyy"}});
    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 2);

    auto chld = mdl.child({}, 0);
    BOOST_CHECK(mdl.text(chld, 0) == L"aaaa");
    BOOST_CHECK(mdl.text(chld, 1) == L"bbb");
    BOOST_CHECK_EQUAL(mdl.childs_size(chld), 0);
    BOOST_CHECK_EQUAL(mdl.index(chld), 0);
    BOOST_CHECK(!mdl.parent(chld).is_valid());

    auto chld2 = mdl.child({}, 1);
    BOOST_CHECK(mdl.text(chld2, 0) == L"xxx");
    BOOST_CHECK(mdl.text(chld2, 1) == L"yyy");
    BOOST_CHECK_EQUAL(mdl.childs_size(chld2), 0);
    BOOST_CHECK_EQUAL(mdl.index(chld2), 1);
    BOOST_CHECK(!mdl.parent(chld2).is_valid());

    BOOST_CHECK_EQUAL(before_added_signum, 1);
    BOOST_CHECK_EQUAL(after_added_signum, 1);
}


/// Tests adding child node
BOOST_AUTO_TEST_CASE(add_child_node) {
    simple_tree_view_model mdl{2, true};

    mdl.insert({}, 0, {{L"root_a", L"root_b"}});
    mdl.insert({}, 1, {{L"root2_a", L"root2_b"}});

    int before_added_signum = 0;
    mdl.before_added().connect([&](auto row, size_t start, size_t end) {
        BOOST_CHECK_EQUAL(before_added_signum, 0);
        BOOST_CHECK(row == mdl.child({}, 1));
        BOOST_CHECK_EQUAL(start, 0);
        BOOST_CHECK_EQUAL(end, 0);

        ++before_added_signum;
    });

    int after_added_signum = 0;
    mdl.after_added().connect([&](auto row, size_t start, size_t end) {
        BOOST_CHECK_EQUAL(after_added_signum, 0);
        BOOST_CHECK(row == mdl.child({}, 1));
        BOOST_CHECK_EQUAL(start, 0);
        BOOST_CHECK_EQUAL(end, 0);

        ++after_added_signum;
    });

    mdl.before_removed().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });

    mdl.after_removed().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });

    mdl.after_changed().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });

}


/// Tests removing root nodeы
BOOST_AUTO_TEST_CASE(remove_root_node) {
    simple_tree_view_model mdl{2, true};

    mdl.insert({}, 0, {{L"root_a", L"root_b"}});
    mdl.insert({}, 1, {{L"root2_a", L"root2_b"}});
    mdl.insert({}, 2, {{L"root3_a", L"root3_b"}});

    mdl.before_added().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });

    mdl.after_added().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });

    int before_removed_signum = 0;
    mdl.before_removed().connect([&](const auto & row, size_t start, size_t end) {
        BOOST_CHECK_EQUAL(before_removed_signum, 0);
        BOOST_CHECK(!row.is_valid());
        BOOST_CHECK_EQUAL(start, 0);
        BOOST_CHECK_EQUAL(end, 1);
        BOOST_CHECK_EQUAL(mdl.childs_size({}), 3);
        ++before_removed_signum;
    });

    int after_removed_signum = 0;
    mdl.after_removed().connect([&](const auto & row, size_t start, size_t end) {
        BOOST_CHECK_EQUAL(after_removed_signum, 0);
        BOOST_CHECK(!row.is_valid());
        BOOST_CHECK_EQUAL(start, 0);
        BOOST_CHECK_EQUAL(end, 1);
        BOOST_CHECK_EQUAL(mdl.childs_size({}), 1);
        ++after_removed_signum;
    });

    mdl.after_changed().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });

    mdl.before_layout_changed().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });

    mdl.after_layout_changed().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });


    mdl.remove({}, 0, 2);
    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 1);

    auto chld = mdl.child({}, 0);
    BOOST_CHECK(mdl.text(chld, 0) == L"root3_a");
    BOOST_CHECK(mdl.text(chld, 1) == L"root3_b");
    BOOST_CHECK_EQUAL(mdl.childs_size(chld), 0);

    BOOST_CHECK_EQUAL(before_removed_signum, 1);
    BOOST_CHECK_EQUAL(after_removed_signum, 1);
}


/// Tests changing root node
BOOST_AUTO_TEST_CASE(change_root_node) {
    simple_tree_view_model mdl{2, true};

    mdl.insert({}, 0, {{L"root_a", L"root_b"}});
    mdl.insert({}, 1, {{L"root2_a", L"root2_b"}});

    mdl.before_added().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });

    mdl.after_added().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });

    mdl.before_removed().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });

    mdl.after_removed().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });

    int after_changed_signum = 0;
    mdl.after_changed().connect([&](const auto & row) {
        BOOST_CHECK_EQUAL(after_changed_signum, 0);
        BOOST_CHECK(row.is_valid());
        BOOST_CHECK_EQUAL(mdl.index(row), 1);
        BOOST_CHECK(!mdl.parent(row).is_valid());
        BOOST_CHECK(mdl.text(row, 0) == L"zzz");

        ++after_changed_signum;
    });

    mdl.before_layout_changed().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });

    mdl.after_layout_changed().connect([](auto && ... pars) {
        BOOST_CHECK(false);
    });


    auto chld = mdl.child({}, 1);
    BOOST_CHECK(mdl.editable(chld, 0));

    mdl.set_text(chld, 0, L"zzz");

    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 2);
    chld = mdl.child({}, 1);
    BOOST_CHECK(mdl.text(chld, 0) == L"zzz");
    BOOST_CHECK(mdl.text(chld, 1) == L"root2_b");
    BOOST_CHECK_EQUAL(mdl.childs_size(chld), 0);

    BOOST_CHECK_EQUAL(after_changed_signum, 1);
}


BOOST_AUTO_TEST_SUITE_END()


}
