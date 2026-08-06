// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file sortable_proxy_tree_view_model.cpp
/// Contains unit tests for the sortable_proxy_tree_view_model class.

#include "../simple_tree_view_model.hpp"
#include "../sortable_proxy_tree_view_model.hpp"
#include <boost/test/unit_test.hpp>
#include <iostream>


namespace cxxdbg::test {


BOOST_AUTO_TEST_SUITE(sortable_proxy_tree_view_model_test)


/// Tests initialization with empty source model
BOOST_AUTO_TEST_CASE(init_empty_src) {
    simple_tree_view_model smdl{2, true};
    smdl.set_column_name(0, L"aaa");
    smdl.set_column_name(1, L"bbb");
    sortable_proxy_tree_view_model mdl{smdl};

    BOOST_CHECK_EQUAL(mdl.columns_size(), 2);
    BOOST_CHECK(mdl.column_name(0) == L"aaa");
    BOOST_CHECK(mdl.column_name(1) == L"bbb");
    BOOST_CHECK_EQUAL(mdl.childs_size({}), 0);
}


/// Tests initialization with not empty model
BOOST_AUTO_TEST_CASE(init) {
    simple_tree_view_model smdl{2, true};
    smdl.set_column_name(0, L"col1");
    smdl.set_column_name(1, L"col2");

    smdl.insert({}, 0, {{L"msd", L"asa"}});
    smdl.insert({}, 1, {{L"asd", L"zsa"}});
    smdl.insert({}, 2, {{L"bvs", L"ksa"}});

    sortable_proxy_tree_view_model mdl{smdl};

    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 3);

    auto child1 = mdl.child({}, 0);
    BOOST_CHECK(mdl.text(child1, 0) == L"msd");
    BOOST_CHECK(mdl.text(child1, 1) == L"asa");
    BOOST_CHECK_EQUAL(mdl.index(child1), 0);

    auto child2 = mdl.child({}, 1);
    BOOST_CHECK(mdl.text(child2, 0) == L"asd");
    BOOST_CHECK(mdl.text(child2, 1) == L"zsa");
    BOOST_CHECK_EQUAL(mdl.index(child2), 1);

    auto child3 = mdl.child({}, 2);
    BOOST_CHECK(mdl.text(child3, 0) == L"bvs");
    BOOST_CHECK(mdl.text(child3, 1) == L"ksa");
    BOOST_CHECK_EQUAL(mdl.index(child3), 2);
}


/// Tests ascending sorting
BOOST_AUTO_TEST_CASE(sort_ascending) {
    simple_tree_view_model smdl{2, true};
    smdl.set_column_name(0, L"col1");
    smdl.set_column_name(1, L"col2");

    smdl.insert({}, 0, {{L"msd", L"asa"}});
    smdl.insert({}, 1, {{L"asd", L"zsa"}});
    smdl.insert({}, 2, {{L"bvs", L"ksa"}});

    sortable_proxy_tree_view_model mdl{smdl};

    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 3);

    mdl.before_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.before_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    int before_layout_changed_signum = 0;
    mdl.before_layout_changed().connect([&](const auto & row) {
        BOOST_CHECK_EQUAL(before_layout_changed_signum, 0);
        BOOST_CHECK(!row.is_valid());
        ++before_layout_changed_signum;
    });

    int after_layout_changed_signum = 0;
    mdl.after_layout_changed().connect([&](const auto & row) {
        BOOST_CHECK_EQUAL(after_layout_changed_signum, 0);
        BOOST_CHECK(!row.is_valid());
        ++after_layout_changed_signum;
    });

    mdl.sort(0, sortable_tree_view_model::sort_order::ascending);

    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 3);

    auto child1 = mdl.child({}, 0);
    BOOST_CHECK(mdl.text(child1, 0) == L"asd");
    BOOST_CHECK(mdl.text(child1, 1) == L"zsa");

    auto child2 = mdl.child({}, 1);
    BOOST_CHECK(mdl.text(child2, 0) == L"bvs");
    BOOST_CHECK(mdl.text(child2, 1) == L"ksa");

    auto child3 = mdl.child({}, 2);
    BOOST_CHECK(mdl.text(child3, 0) == L"msd");
    BOOST_CHECK(mdl.text(child3, 1) == L"asa");

    BOOST_CHECK_EQUAL(before_layout_changed_signum, 1);
    BOOST_CHECK_EQUAL(after_layout_changed_signum, 1);
}


/// Tests descending sorting
BOOST_AUTO_TEST_CASE(sort_descending) {
    simple_tree_view_model smdl{2, true};
    smdl.set_column_name(0, L"col1");
    smdl.set_column_name(1, L"col2");

    smdl.insert({}, 0, {{L"msd", L"asa"}});
    smdl.insert({}, 1, {{L"asd", L"zsa"}});
    smdl.insert({}, 2, {{L"bvs", L"ksa"}});

    sortable_proxy_tree_view_model mdl{smdl};

    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 3);

    mdl.before_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.before_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    int before_layout_changed_signum = 0;
    mdl.before_layout_changed().connect([&](const auto & row) {
        BOOST_CHECK(before_layout_changed_signum == 0);
        BOOST_CHECK(!row.is_valid());
        ++before_layout_changed_signum;
    });

    int after_layout_changed_signum = 0;
    mdl.after_layout_changed().connect([&](const auto & row) {
        BOOST_CHECK(after_layout_changed_signum == 0);
        BOOST_CHECK(!row.is_valid());
        ++after_layout_changed_signum;
    });


    mdl.sort(0, sortable_tree_view_model::sort_order::descending);

    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 3);

    auto child1 = mdl.child({}, 0);
    BOOST_CHECK(mdl.text(child1, 0) == L"msd");
    BOOST_CHECK(mdl.text(child1, 1) == L"asa");

    auto child2 = mdl.child({}, 1);
    BOOST_CHECK(mdl.text(child2, 0) == L"bvs");
    BOOST_CHECK(mdl.text(child2, 1) == L"ksa");

    auto child3 = mdl.child({}, 2);
    BOOST_CHECK(mdl.text(child3, 0) == L"asd");
    BOOST_CHECK(mdl.text(child3, 1) == L"zsa");

    BOOST_CHECK_EQUAL(before_layout_changed_signum, 1);
    BOOST_CHECK_EQUAL(after_layout_changed_signum, 1);
}


/// Tests adding items with no sorting
BOOST_AUTO_TEST_CASE(add_no_sort) {
    simple_tree_view_model smdl{2, true};
    smdl.set_column_name(0, L"col1");
    smdl.set_column_name(1, L"col2");

    smdl.insert({}, 0, {{L"msd", L"asa"}});
    smdl.insert({}, 1, {{L"asd", L"zsa"}});

    sortable_proxy_tree_view_model mdl{smdl};

    int before_added_signum = 0;
    mdl.before_added().connect([&](const auto & row_index, size_t start, size_t end) {
        BOOST_CHECK_EQUAL(before_added_signum, 0);
        BOOST_CHECK_EQUAL(start, 1);
        BOOST_CHECK_EQUAL(end, 2);
        BOOST_CHECK(!row_index.is_valid());
        BOOST_CHECK_EQUAL(mdl.childs_size({}), 2);
        ++before_added_signum;
    });

    int after_added_signum = 0;
    mdl.after_added().connect([&](const auto & row_index, size_t start, size_t end) {
        BOOST_CHECK_EQUAL(after_added_signum, 0);
        BOOST_CHECK_EQUAL(start, 1);
        BOOST_CHECK_EQUAL(end, 2);
        BOOST_CHECK(!row_index.is_valid());
        BOOST_CHECK_EQUAL(mdl.childs_size({}), 4);
        ++after_added_signum;
    });

    mdl.before_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.before_layout_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_layout_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    smdl.insert({}, 1, {{L"bvs", L"ksa"}, {L"bas", L"kkk"}});

    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 4);

    auto child1 = mdl.child({}, 0);
    BOOST_CHECK(mdl.text(child1, 0) == L"msd");
    BOOST_CHECK(mdl.text(child1, 1) == L"asa");

    auto child2 = mdl.child({}, 1);
    BOOST_CHECK(mdl.text(child2, 0) == L"bvs");
    BOOST_CHECK(mdl.text(child2, 1) == L"ksa");

    auto child3 = mdl.child({}, 2);
    BOOST_CHECK(mdl.text(child3, 0) == L"bas");
    BOOST_CHECK(mdl.text(child3, 1) == L"kkk");

    auto child4 = mdl.child({}, 3);
    BOOST_CHECK(mdl.text(child4, 0) == L"asd");
    BOOST_CHECK(mdl.text(child4, 1) == L"zsa");

    BOOST_CHECK_EQUAL(before_added_signum, 1);
    BOOST_CHECK_EQUAL(after_added_signum, 1);
}


/// Tests adding items with ascending sorting
BOOST_AUTO_TEST_CASE(add_ascending) {
    simple_tree_view_model smdl{2, true};
    smdl.set_column_name(0, L"col1");
    smdl.set_column_name(1, L"col2");

    smdl.insert({}, 0, {{L"msd", L"asa"}});
    smdl.insert({}, 1, {{L"asd", L"zsa"}});

    sortable_proxy_tree_view_model mdl{smdl};
    mdl.sort(0, sortable_tree_view_model::sort_order::ascending);

    int before_added_signum = 0;
    mdl.before_added().connect([&](const auto & row_index, size_t start, size_t end) {
        if (before_added_signum == 0) {
            BOOST_CHECK_EQUAL(start, 1);
            BOOST_CHECK_EQUAL(end, 1);
            BOOST_CHECK(!row_index.is_valid());

            BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 2);

            auto child1 = mdl.child({}, 0);
            BOOST_CHECK(mdl.text(child1, 0) == L"asd");

            auto child2 = mdl.child({}, 1);
            BOOST_CHECK(mdl.text(child2, 0) == L"msd");

        } else if (before_added_signum == 1) {
            BOOST_CHECK_EQUAL(start, 1);
            BOOST_CHECK_EQUAL(end, 1);
            BOOST_CHECK(!row_index.is_valid());

            BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 3);

            auto child1 = mdl.child({}, 0);
            BOOST_CHECK(mdl.text(child1, 0) == L"asd");

            auto child2 = mdl.child({}, 1);
            BOOST_CHECK(mdl.text(child2, 0) == L"bvs");

            auto child3 = mdl.child({}, 2);
            BOOST_CHECK(mdl.text(child3, 0) == L"msd");
        } else {
            BOOST_CHECK(false);
        }

        ++before_added_signum;
    });

    int after_added_signum = 0;
    mdl.after_added().connect([&](const auto & row_index, size_t start, size_t end) {
        if (after_added_signum == 0) {
            BOOST_CHECK_EQUAL(start, 1);
            BOOST_CHECK_EQUAL(end, 1);
            BOOST_CHECK(!row_index.is_valid());

            BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 3);

            auto child1 = mdl.child({}, 0);
            BOOST_CHECK(mdl.text(child1, 0) == L"asd");

            auto child2 = mdl.child({}, 1);
            BOOST_CHECK(mdl.text(child2, 0) == L"bvs");

            auto child3 = mdl.child({}, 2);
            BOOST_CHECK(mdl.text(child3, 0) == L"msd");

        } else if (after_added_signum == 1) {
            BOOST_CHECK_EQUAL(start, 1);
            BOOST_CHECK_EQUAL(end, 1);
            BOOST_CHECK(!row_index.is_valid());

            BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 4);

            auto child1 = mdl.child({}, 0);
            BOOST_CHECK(mdl.text(child1, 0) == L"asd");

            auto child2 = mdl.child({}, 1);
            BOOST_CHECK(mdl.text(child2, 0) == L"bas");

            auto child3 = mdl.child({}, 2);
            BOOST_CHECK(mdl.text(child3, 0) == L"bvs");

            auto child4 = mdl.child({}, 3);
            BOOST_CHECK(mdl.text(child4, 0) == L"msd");
        } else {
            BOOST_CHECK(false);
        }

        ++after_added_signum;
    });

    mdl.before_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.before_layout_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_layout_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    smdl.insert({}, 1, {{L"bvs", L"ksa"}, {L"bas", L"kkk"}});

    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 4);

    auto child1 = mdl.child({}, 0);
    BOOST_CHECK(mdl.text(child1, 0) == L"asd");
    BOOST_CHECK(mdl.text(child1, 1) == L"zsa");

    auto child2 = mdl.child({}, 1);
    BOOST_CHECK(mdl.text(child2, 0) == L"bas");
    BOOST_CHECK(mdl.text(child2, 1) == L"kkk");

    auto child3 = mdl.child({}, 2);
    BOOST_CHECK(mdl.text(child3, 0) == L"bvs");
    BOOST_CHECK(mdl.text(child3, 1) == L"ksa");

    auto child4 = mdl.child({}, 3);
    BOOST_CHECK(mdl.text(child4, 0) == L"msd");
    BOOST_CHECK(mdl.text(child4, 1) == L"asa");

    BOOST_CHECK_EQUAL(before_added_signum, 2);
    BOOST_CHECK_EQUAL(after_added_signum, 2);
}


/// Tests adding items with descending sorting
BOOST_AUTO_TEST_CASE(add_descending) {
    simple_tree_view_model smdl{2, true};
    smdl.set_column_name(0, L"col1");
    smdl.set_column_name(1, L"col2");

    smdl.insert({}, 0, {{L"msd", L"asa"}});
    smdl.insert({}, 1, {{L"asd", L"zsa"}});

    sortable_proxy_tree_view_model mdl{smdl};
    mdl.sort(0, sortable_tree_view_model::sort_order::descending);

    int before_added_signum = 0;
    mdl.before_added().connect([&](const auto & row_index, size_t start, size_t end) {
        if (before_added_signum == 0) {
            BOOST_CHECK_EQUAL(start, 1);
            BOOST_CHECK_EQUAL(end, 1);
            BOOST_CHECK(!row_index.is_valid());

            BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 2);

            auto child1 = mdl.child({}, 0);
            BOOST_CHECK(mdl.text(child1, 0) == L"msd");

            auto child2 = mdl.child({}, 1);
            BOOST_CHECK(mdl.text(child2, 0) == L"asd");

        } else if (before_added_signum == 1) {
            BOOST_CHECK_EQUAL(start, 0);
            BOOST_CHECK_EQUAL(end, 0);
            BOOST_CHECK(!row_index.is_valid());

            BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 3);

            auto child1 = mdl.child({}, 0);
            BOOST_CHECK(mdl.text(child1, 0) == L"msd");

            auto child2 = mdl.child({}, 1);
            BOOST_CHECK(mdl.text(child2, 0) == L"bvs");

            auto child3 = mdl.child({}, 2);
            BOOST_CHECK(mdl.text(child3, 0) == L"asd");
        } else {
            BOOST_CHECK(false);
        }

        ++before_added_signum;
    });

    int after_added_signum = 0;
    mdl.after_added().connect([&](const auto & row_index, size_t start, size_t end) {
        if (after_added_signum == 0) {
            BOOST_CHECK_EQUAL(start, 1);
            BOOST_CHECK_EQUAL(end, 1);
            BOOST_CHECK(!row_index.is_valid());

            BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 3);

            auto child1 = mdl.child({}, 0);
            BOOST_CHECK(mdl.text(child1, 0) == L"msd");

            auto child2 = mdl.child({}, 1);
            BOOST_CHECK(mdl.text(child2, 0) == L"bvs");

            auto child3 = mdl.child({}, 2);
            BOOST_CHECK(mdl.text(child3, 0) == L"asd");

        } else if (after_added_signum == 1) {
            BOOST_CHECK_EQUAL(start, 0);
            BOOST_CHECK_EQUAL(end, 0);
            BOOST_CHECK(!row_index.is_valid());

            BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 4);

            auto child1 = mdl.child({}, 0);
            BOOST_CHECK(mdl.text(child1, 0) == L"zas");

            auto child2 = mdl.child({}, 1);
            BOOST_CHECK(mdl.text(child2, 0) == L"msd");

            auto child3 = mdl.child({}, 2);
            BOOST_CHECK(mdl.text(child3, 0) == L"bvs");

            auto child4 = mdl.child({}, 3);
            BOOST_CHECK(mdl.text(child4, 0) == L"asd");

        } else {
            BOOST_CHECK(false);
        }

        ++after_added_signum;
    });

    mdl.before_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.before_layout_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_layout_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    smdl.insert({}, 1, {{L"bvs", L"ksa"}, {L"zas", L"kkk"}});

    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 4);

    auto child1 = mdl.child({}, 0);
    BOOST_CHECK(mdl.text(child1, 0) == L"zas");
    BOOST_CHECK(mdl.text(child1, 1) == L"kkk");

    auto child2 = mdl.child({}, 1);
    BOOST_CHECK(mdl.text(child2, 0) == L"msd");
    BOOST_CHECK(mdl.text(child2, 1) == L"asa");

    auto child3 = mdl.child({}, 2);
    BOOST_CHECK(mdl.text(child3, 0) == L"bvs");
    BOOST_CHECK(mdl.text(child3, 1) == L"ksa");

    auto child4 = mdl.child({}, 3);
    BOOST_CHECK(mdl.text(child4, 0) == L"asd");
    BOOST_CHECK(mdl.text(child4, 1) == L"zsa");

    BOOST_CHECK_EQUAL(before_added_signum, 2);
    BOOST_CHECK_EQUAL(after_added_signum, 2);
}


/// Tests removing items with no sorting
BOOST_AUTO_TEST_CASE(remove_no_sort) {
    simple_tree_view_model smdl{2, true};
    smdl.set_column_name(0, L"col1");
    smdl.set_column_name(1, L"col2");

    smdl.insert({}, 0, {{L"msd", L"asa"}});
    smdl.insert({}, 1, {{L"asd", L"zsa"}});
    smdl.insert({}, 2, {{L"bvs", L"ksa"}});
    smdl.insert({}, 3, {{L"zas", L"kkk"}});

    sortable_proxy_tree_view_model mdl{smdl};

    mdl.before_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    int before_removed_signum = 0;
    mdl.before_removed().connect([&](const auto & row_index, size_t start, size_t end) {
        BOOST_CHECK_EQUAL(before_removed_signum, 0);
        BOOST_CHECK_EQUAL(start, 2);
        BOOST_CHECK_EQUAL(end, 3);
        BOOST_CHECK(!row_index.is_valid());

        BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 4);

        auto child1 = mdl.child({}, 0);
        BOOST_CHECK(mdl.text(child1, 0) == L"msd");

        auto child2 = mdl.child({}, 1);
        BOOST_CHECK(mdl.text(child2, 0) == L"asd");

        auto child3 = mdl.child({}, 2);
        BOOST_CHECK(mdl.text(child3, 0) == L"bvs");

        auto child4 = mdl.child({}, 3);
        BOOST_CHECK(mdl.text(child4, 0) == L"zas");

        ++before_removed_signum;
    });

    int after_removed_signum = 0;
    mdl.after_removed().connect([&](const auto & row_index, size_t start, size_t end) {
        BOOST_CHECK_EQUAL(after_removed_signum, 0);
        BOOST_CHECK_EQUAL(start, 2);
        BOOST_CHECK_EQUAL(end, 3);
        BOOST_CHECK(!row_index.is_valid());

        BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 2);

        auto child1 = mdl.child({}, 0);
        BOOST_CHECK(mdl.text(child1, 0) == L"msd");

        auto child2 = mdl.child({}, 1);
        BOOST_CHECK(mdl.text(child2, 0) == L"asd");
        ++after_removed_signum;
    });

    mdl.after_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.before_layout_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_layout_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    smdl.remove({}, 2, 2);

    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 2);

    auto child1 = mdl.child({}, 0);
    BOOST_CHECK(mdl.text(child1, 0) == L"msd");
    BOOST_CHECK(mdl.text(child1, 1) == L"asa");

    auto child2 = mdl.child({}, 1);
    BOOST_CHECK(mdl.text(child2, 0) == L"asd");
    BOOST_CHECK(mdl.text(child2, 1) == L"zsa");


    BOOST_CHECK_EQUAL(before_removed_signum, 1);
    BOOST_CHECK_EQUAL(after_removed_signum, 1);
}


/// Tests removing items with ascending sorting
BOOST_AUTO_TEST_CASE(remove_ascending) {
    simple_tree_view_model smdl{2, true};
    smdl.set_column_name(0, L"col1");
    smdl.set_column_name(1, L"col2");

    smdl.insert({}, 0, {{L"msd", L"asa"}});
    smdl.insert({}, 1, {{L"asd", L"zsa"}});
    smdl.insert({}, 2, {{L"bvs", L"ksa"}});
    smdl.insert({}, 3, {{L"zas", L"kkk"}});

    sortable_proxy_tree_view_model mdl{smdl};
    mdl.sort(0, sortable_tree_view_model::sort_order::ascending);

    mdl.before_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    int before_removed_signum = 0;
    mdl.before_removed().connect([&](const auto & row_index, size_t start, size_t end) {
        if (before_removed_signum == 0) {
            BOOST_CHECK_EQUAL(start, 1);
            BOOST_CHECK_EQUAL(end, 1);
            BOOST_CHECK(!row_index.is_valid());

            BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 4);

            auto child1 = mdl.child({}, 0);
            BOOST_CHECK(mdl.text(child1, 0) == L"asd");

            auto child2 = mdl.child({}, 1);
            BOOST_CHECK(mdl.text(child2, 0) == L"bvs");

            auto child3 = mdl.child({}, 2);
            BOOST_CHECK(mdl.text(child3, 0) == L"msd");

            auto child4 = mdl.child({}, 3);
            BOOST_CHECK(mdl.text(child4, 0) == L"zas");

        } else if (before_removed_signum == 1) {

            BOOST_CHECK_EQUAL(start, 2);
            BOOST_CHECK_EQUAL(end, 2);
            BOOST_CHECK(!row_index.is_valid());

            BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 3);

            auto child1 = mdl.child({}, 0);
            BOOST_CHECK(mdl.text(child1, 0) == L"asd");

            auto child2 = mdl.child({}, 1);
            BOOST_CHECK(mdl.text(child2, 0) == L"msd");

            auto child3 = mdl.child({}, 2);
            BOOST_CHECK(mdl.text(child3, 0) == L"zas");

        } else {
            BOOST_CHECK(false);
        }

        ++before_removed_signum;
    });

    int after_removed_signum = 0;
    mdl.after_removed().connect([&](const auto & row_index, size_t start, size_t end) {
        if (after_removed_signum == 0) {
            BOOST_CHECK_EQUAL(start, 1);
            BOOST_CHECK_EQUAL(end, 1);
            BOOST_CHECK(!row_index.is_valid());

            BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 3);

            auto child1 = mdl.child({}, 0);
            BOOST_CHECK(mdl.text(child1, 0) == L"asd");

            auto child2 = mdl.child({}, 1);
            BOOST_CHECK(mdl.text(child2, 0) == L"msd");

            auto child3 = mdl.child({}, 2);
            BOOST_CHECK(mdl.text(child3, 0) == L"zas");

        } else if (after_removed_signum == 1) {

            BOOST_CHECK_EQUAL(start, 2);
            BOOST_CHECK_EQUAL(end, 2);
            BOOST_CHECK(!row_index.is_valid());

            BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 2);

            auto child1 = mdl.child({}, 0);
            BOOST_CHECK(mdl.text(child1, 0) == L"asd");

            auto child2 = mdl.child({}, 1);
            BOOST_CHECK(mdl.text(child2, 0) == L"msd");

        } else {
            BOOST_CHECK(false);
        }

        ++after_removed_signum;
    });

    mdl.after_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.before_layout_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_layout_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    smdl.remove({}, 2, 2);

    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 2);

    auto child1 = mdl.child({}, 0);
    BOOST_CHECK(mdl.text(child1, 0) == L"asd");
    BOOST_CHECK(mdl.text(child1, 1) == L"zsa");

    auto child2 = mdl.child({}, 1);
    BOOST_CHECK(mdl.text(child2, 0) == L"msd");
    BOOST_CHECK(mdl.text(child2, 1) == L"asa");

    BOOST_CHECK_EQUAL(before_removed_signum, 2);
    BOOST_CHECK_EQUAL(after_removed_signum, 2);
}


/// Tests removing items with descending sorting
BOOST_AUTO_TEST_CASE(remove_descending) {
    simple_tree_view_model smdl{2, true};
    smdl.set_column_name(0, L"col1");
    smdl.set_column_name(1, L"col2");

    smdl.insert({}, 0, {{L"msd", L"asa"}});
    smdl.insert({}, 1, {{L"asd", L"zsa"}});
    smdl.insert({}, 2, {{L"bvs", L"ksa"}});
    smdl.insert({}, 3, {{L"zas", L"kkk"}});

    sortable_proxy_tree_view_model mdl{smdl};
    mdl.sort(0, sortable_tree_view_model::sort_order::descending);

    mdl.before_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    int before_removed_signum = 0;
    mdl.before_removed().connect([&](const auto & row_index, size_t start, size_t end) {
        if (before_removed_signum == 0) {
            BOOST_CHECK_EQUAL(start, 2);
            BOOST_CHECK_EQUAL(end, 2);
            BOOST_CHECK(!row_index.is_valid());

            BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 4);

            auto child1 = mdl.child({}, 0);
            BOOST_CHECK(mdl.text(child1, 0) == L"zas");

            auto child2 = mdl.child({}, 1);
            BOOST_CHECK(mdl.text(child2, 0) == L"msd");

            auto child3 = mdl.child({}, 2);
            BOOST_CHECK(mdl.text(child3, 0) == L"bvs");

            auto child4 = mdl.child({}, 3);
            BOOST_CHECK(mdl.text(child4, 0) == L"asd");

        } else if (before_removed_signum == 1) {

            BOOST_CHECK_EQUAL(start, 0);
            BOOST_CHECK_EQUAL(end, 0);
            BOOST_CHECK(!row_index.is_valid());

            BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 3);

            auto child1 = mdl.child({}, 0);
            BOOST_CHECK(mdl.text(child1, 0) == L"zas");

            auto child2 = mdl.child({}, 1);
            BOOST_CHECK(mdl.text(child2, 0) == L"msd");

            auto child3 = mdl.child({}, 2);
            BOOST_CHECK(mdl.text(child3, 0) == L"asd");

        } else {
            BOOST_CHECK(false);
        }

        ++before_removed_signum;
    });

    int after_removed_signum = 0;
    mdl.after_removed().connect([&](const auto & row_index, size_t start, size_t end) {
        if (after_removed_signum == 0) {
            BOOST_CHECK_EQUAL(start, 2);
            BOOST_CHECK_EQUAL(end, 2);
            BOOST_CHECK(!row_index.is_valid());

            BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 3);

            auto child1 = mdl.child({}, 0);
            BOOST_CHECK(mdl.text(child1, 0) == L"zas");

            auto child2 = mdl.child({}, 1);
            BOOST_CHECK(mdl.text(child2, 0) == L"msd");

            auto child3 = mdl.child({}, 2);
            BOOST_CHECK(mdl.text(child3, 0) == L"asd");

        } else if (after_removed_signum == 1) {

            BOOST_CHECK_EQUAL(start, 0);
            BOOST_CHECK_EQUAL(end, 0);
            BOOST_CHECK(!row_index.is_valid());

            BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 2);

            auto child1 = mdl.child({}, 0);
            BOOST_CHECK(mdl.text(child1, 0) == L"msd");

            auto child2 = mdl.child({}, 1);
            BOOST_CHECK(mdl.text(child2, 0) == L"asd");

        } else {
            BOOST_CHECK(false);
        }

        ++after_removed_signum;
    });

    mdl.after_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.before_layout_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_layout_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    smdl.remove({}, 2, 2);

    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 2);

    auto child1 = mdl.child({}, 0);
    BOOST_CHECK(mdl.text(child1, 0) == L"msd");
    BOOST_CHECK(mdl.text(child1, 1) == L"asa");

    auto child2 = mdl.child({}, 1);
    BOOST_CHECK(mdl.text(child2, 0) == L"asd");
    BOOST_CHECK(mdl.text(child2, 1) == L"zsa");

    BOOST_CHECK_EQUAL(before_removed_signum, 2);
    BOOST_CHECK_EQUAL(after_removed_signum, 2);
}


/// Tests adding not root items
BOOST_AUTO_TEST_CASE(add_no_root) {
    simple_tree_view_model smdl{2, true};
    smdl.set_column_name(0, L"col1");
    smdl.set_column_name(1, L"col2");

    smdl.insert({}, 0, {{L"msd", L"asa"}});

    sortable_proxy_tree_view_model mdl{smdl};
    mdl.sort(0, sortable_tree_view_model::sort_order::ascending);

    int before_added_signum = 0;
    mdl.before_added().connect([&](const auto & row_index, size_t start, size_t end) {
        BOOST_CHECK_EQUAL(before_added_signum, 0);
        BOOST_CHECK_EQUAL(start, 0);
        BOOST_CHECK_EQUAL(end, 1);
        BOOST_CHECK(row_index.is_valid());
        ++before_added_signum;
    });

    int after_added_signum = 0;
    mdl.after_added().connect([&](const auto & row_index, size_t start, size_t end) {
        BOOST_CHECK_EQUAL(after_added_signum, 0);
        BOOST_CHECK_EQUAL(start, 0);
        BOOST_CHECK_EQUAL(end, 1);
        BOOST_CHECK(row_index.is_valid());
        ++after_added_signum;
    });

    mdl.before_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.before_layout_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_layout_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    smdl.insert(smdl.child({}, 0), 0, {{L"bvs", L"ksa"}, {L"bas", L"kkk"}});

    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 1);
    auto node = mdl.child({}, 0);
    BOOST_REQUIRE(node.is_valid());

    BOOST_REQUIRE_EQUAL(mdl.childs_size(node), 2);

    auto child1 = mdl.child(node, 0);
    BOOST_CHECK(mdl.text(child1, 0) == L"bvs");
    BOOST_CHECK(mdl.text(child1, 1) == L"ksa");

    auto child2 = mdl.child(node, 1);
    BOOST_CHECK(mdl.text(child2, 0) == L"bas");
    BOOST_CHECK(mdl.text(child2, 1) == L"kkk");

    BOOST_CHECK_EQUAL(before_added_signum, 1);
    BOOST_CHECK_EQUAL(after_added_signum, 1);
}


/// Tests removing not root items
BOOST_AUTO_TEST_CASE(remove_no_root) {
    simple_tree_view_model smdl{2, true};
    smdl.set_column_name(0, L"col1");
    smdl.set_column_name(1, L"col2");

    smdl.insert({}, 0, {{L"msd", L"asa"}});
    smdl.insert(smdl.child({}, 0), 0, {{L"bvs", L"ksa"}, {L"bas", L"kkk"}});

    sortable_proxy_tree_view_model mdl{smdl};
    mdl.sort(0, sortable_tree_view_model::sort_order::ascending);

    mdl.before_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    int before_removed_signum = 0;
    mdl.before_removed().connect([&](const auto & row_index, size_t start, size_t end) {
        BOOST_CHECK_EQUAL(before_removed_signum, 0);
        BOOST_CHECK(row_index.is_valid());
        BOOST_CHECK_EQUAL(start, 0);
        BOOST_CHECK_EQUAL(end, 0);
        BOOST_CHECK_EQUAL(mdl.childs_size(row_index), 2);
        ++before_removed_signum;
    });

    int after_removed_signum = 0;
    mdl.after_removed().connect([&](const auto & row_index, size_t start, size_t end) {
        BOOST_CHECK_EQUAL(after_removed_signum, 0);
        BOOST_CHECK(row_index.is_valid());
        BOOST_CHECK_EQUAL(start, 0);
        BOOST_CHECK_EQUAL(end, 0);
        BOOST_CHECK_EQUAL(mdl.childs_size(row_index), 1);
        ++after_removed_signum;
    });

    mdl.after_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.before_layout_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    mdl.after_layout_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    smdl.remove(smdl.child({}, 0), 0);


    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 1);
    auto node = mdl.child({}, 0);
    BOOST_REQUIRE(node.is_valid());

    BOOST_REQUIRE_EQUAL(mdl.childs_size(node), 1);

    auto child1 = mdl.child(node, 0);
    BOOST_CHECK(mdl.text(child1, 0) == L"bas");
    BOOST_CHECK(mdl.text(child1, 1) == L"kkk");

    BOOST_CHECK_EQUAL(before_removed_signum, 1);
    BOOST_CHECK_EQUAL(after_removed_signum, 1);
}



BOOST_AUTO_TEST_SUITE_END()


}
