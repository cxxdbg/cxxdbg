// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

//
// Created by yzarudny on 20.10.17.
//

#include "../breakpoint_list.hpp"
#include "../code_breakpoint.hpp"
#include "cxxdbg/dbg/source_model.hpp"
#include "cxxdbg/dbg/watchpoint.hpp"
#include "cxxdbg/dbg/breakpoints_view_model.hpp"
#include <iostream>
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::test {

namespace {

class mock_breakpoint: public code_breakpoint {
public:
    mock_breakpoint(source_model & src_mdl, num_t n):
            code_breakpoint{src_mdl, n} {}

    ~mock_breakpoint() override {}

    std::string name() const override {
        return "mock_bp_name";
    }

    void install_into_impl(breakpoint_list_impl &, const install_handler & h) override {
        h(code_breakpoint_info{-1});
    }
};

}

struct breakpoints_model_fixture {
    source_model src_mdl;
    breakpoint_list bplist{src_mdl};
    breakpoints_view_model model{bplist};
    tree_view_model_row_index nullindex{breakpoints_view_model::make_index(nullptr)};
};

BOOST_FIXTURE_TEST_SUITE(breakpoints_model_test, breakpoints_model_fixture)

BOOST_AUTO_TEST_CASE(state_of_empty_model) {
    BOOST_CHECK(model.childs_size(nullindex) == 0);

    BOOST_CHECK(model.columns_size() == 5);

    BOOST_CHECK(model.column_name(0) == L"Name");
    BOOST_CHECK(model.column_name(1) == L"Condition");
    BOOST_CHECK(model.column_name(2) == L"Hit count");
    BOOST_CHECK(model.column_name(3) == L"Function");
    BOOST_CHECK(model.column_name(4) == L"Address");
}

BOOST_AUTO_TEST_CASE(add_initial_root_breakpoint) {
    mock_breakpoint bp(src_mdl, 10);
    // add locations
    code_position cpos{10, 20, "func", source_position{}};
    std::shared_ptr<breakpoint_location> loc{new breakpoint_location{&bp, 11, cpos}};
    bp.add_location(loc);

    int n_before_added = 0;
    model.before_added().connect([this, &n_before_added](const auto & parent, auto first, auto last) {
        ++n_before_added;
        BOOST_CHECK(!parent.is_valid());
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 0);
    });

    int n_after_added = 0;
    model.after_added().connect([this, &n_after_added](const auto & parent, auto first, auto last) {
        ++n_after_added;
        BOOST_CHECK(!parent.is_valid());
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 0);
    });

    model.before_removed().connect([](const auto & parent, auto first, auto last) {
        BOOST_CHECK(false);
    });

    model.after_removed().connect([](const auto & parent, auto first, auto last) {
        BOOST_CHECK(false);
    });

    model.after_changed().connect([](const auto & row) {
        BOOST_CHECK(false);
    });

    model.on_breakpoint_added(&bp);

    BOOST_CHECK(model.childs_size(nullindex) == 1);
    auto root_index = model.child(nullindex, 0);
    BOOST_REQUIRE(root_index.is_valid());
    auto * bp_node = breakpoints_view_model::make_tree_node(root_index);
    BOOST_CHECK(bp_node->parent() == nullptr);
    BOOST_CHECK(model.childs_size(root_index) == 0);    // one location means no children
    BOOST_CHECK_EQUAL(n_before_added, 1);
    BOOST_CHECK_EQUAL(n_after_added, 1);
}

BOOST_AUTO_TEST_CASE(add_another_breakpoint) {
    mock_breakpoint bp(src_mdl, 10);
    bp.set_enabled(true);
    bp.set_condition("false");
    // add location
    code_position cpos{10, 20, "func", source_position{}};
    std::shared_ptr<breakpoint_location> loc{new breakpoint_location{&bp, 11, cpos}};
    bp.add_location(loc);

    model.on_breakpoint_added(&bp);

    BOOST_CHECK(model.childs_size(nullindex) == 1); // 1 root node
    auto bp1_index = model.child(nullindex, 0);
    BOOST_REQUIRE(bp1_index.is_valid());

    // check bp1 (root node 1)
    BOOST_CHECK(model.text(bp1_index, 0) == L"mock_bp_name (func)");
    BOOST_CHECK(model.text(bp1_index, 1) == L"false");
    BOOST_CHECK(model.text(bp1_index, 2) == L"Break always (currently 0)");
    BOOST_CHECK(model.text(bp1_index, 3) == L"func");
    BOOST_CHECK(model.text(bp1_index, 4) == L"0x0000000A");

    // add second bp
    mock_breakpoint bp2(src_mdl, 12);
    bp2.set_enabled(true);
    bp2.set_condition("1");
    // add location
    code_position cpos2{30, 50, "func2", source_position{}};
    std::shared_ptr<breakpoint_location> loc2{new breakpoint_location{&bp2, 13, cpos2}};
    bp2.add_location(loc2);

    int n_before_added = 0;
    model.before_added().connect([this, &n_before_added](const auto & parent, auto first, auto last) {
        ++n_before_added;
        BOOST_CHECK(!parent.is_valid());
        BOOST_CHECK_EQUAL(first, 1);
        BOOST_CHECK_EQUAL(last, 1);
    });

    int n_after_added = 0;
    model.after_added().connect([this, &n_after_added](const auto & parent, auto first, auto last) {
        ++n_after_added;
        BOOST_CHECK(!parent.is_valid());
        BOOST_CHECK_EQUAL(first, 1);
        BOOST_CHECK_EQUAL(last, 1);
    });

    model.before_removed().connect([](const auto & parent, auto first, auto last) {
        BOOST_CHECK(false);
    });

    model.after_removed().connect([](const auto & parent, auto first, auto last) {
        BOOST_CHECK(false);
    });

    model.after_changed().connect([](const auto & row) {
        BOOST_CHECK(false);
    });

    model.on_breakpoint_added(&bp2);

    BOOST_CHECK(model.childs_size(nullindex) == 2); // 2 root nodes

    bp1_index = model.child(nullindex, 0);  // update bp1_index
    auto bp2_index = model.child(nullindex, 1);

    BOOST_REQUIRE(bp1_index.is_valid());
    BOOST_REQUIRE(bp2_index.is_valid());

    // recheck bp1 (root node 1)
    BOOST_CHECK(model.text(bp1_index, 0) == L"mock_bp_name (func)");
    BOOST_CHECK(model.text(bp1_index, 1) == L"false");
    BOOST_CHECK(model.text(bp1_index, 2) == L"Break always (currently 0)");
    BOOST_CHECK(model.text(bp1_index, 3) == L"func");
    BOOST_CHECK(model.text(bp1_index, 4) == L"0x0000000A");

    // check bp2 (root node 2)
    BOOST_CHECK(model.text(bp2_index, 0) == L"mock_bp_name (func2)");
    BOOST_CHECK(model.text(bp2_index, 1) == L"1");
    BOOST_CHECK(model.text(bp2_index, 2) == L"Break always (currently 0)");
    BOOST_CHECK(model.text(bp2_index, 3) == L"func2");
    BOOST_CHECK(model.text(bp2_index, 4) == L"0x0000001E");


    BOOST_CHECK(model.childs_size(bp1_index) == 0);    // one location means no children
    BOOST_CHECK(model.childs_size(bp2_index) == 0);    // one location means no children

    BOOST_CHECK_EQUAL(n_before_added, 1);
    BOOST_CHECK_EQUAL(n_after_added, 1);
}

BOOST_AUTO_TEST_CASE(update_breakpoint) {
    mock_breakpoint bp(src_mdl, 10);
    bp.set_enabled(true);
    bp.set_condition("false");

    // add location
    code_position cpos{10, 20, "func", source_position{}};
    std::shared_ptr<breakpoint_location> loc{new breakpoint_location{&bp, 11, cpos}};
    bp.add_location(loc);

    model.on_breakpoint_added(&bp);

    auto root_index = model.child(nullindex, 0);
    BOOST_CHECK(model.text(root_index, 0) == L"mock_bp_name (func)");
    BOOST_CHECK(model.text(root_index, 1) == L"false");
    BOOST_CHECK(model.text(root_index, 2) == L"Break always (currently 0)");
    BOOST_CHECK(model.text(root_index, 3) == L"func");
    BOOST_CHECK(model.text(root_index, 4) == L"0x0000000A");

    BOOST_CHECK(model.childs_size(nullindex) == 1);

    BOOST_CHECK(root_index.is_valid());
    auto * bp_node = breakpoints_view_model::make_tree_node(root_index);
    BOOST_CHECK(bp_node->parent() == nullptr);
    BOOST_CHECK(model.childs_size(root_index) == 0);    // one location means no children

    bp.set_enabled(false);
    bp.set_condition("true");
    {
        code_breakpoint_info hit_info{10};
        hit_info.set_curr_hit_count(1);
        bp.breakpoint::update(&hit_info);
    }

    code_position cpos2{30, 50, "func2", source_position{}};
    std::shared_ptr<breakpoint_location> loc2{new breakpoint_location{&bp, 12, cpos2}};
    bp.add_location(loc2);

    int n_before_added = 0;
    model.before_added().connect([this, &n_before_added](const auto & parent, auto first, auto last) {
        ++n_before_added;
        BOOST_REQUIRE(parent.is_valid());
        BOOST_CHECK_EQUAL(model.index(parent), 0);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 1);
    });

    int n_after_added = 0;
    model.after_added().connect([this, &n_after_added](const auto & parent, auto first, auto last) {
        ++n_after_added;
        BOOST_REQUIRE(parent.is_valid());
        BOOST_CHECK_EQUAL(model.index(parent), 0);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 1);
    });

    model.before_removed().connect([](const auto & parent, auto first, auto last) {
        std::wcerr << "BEFORE REMOVED prent valid = " << (int)parent.is_valid() << "\n";
        std::wcerr << "BEFORE REMOVED first = " << first << ", last = " << last << "\n";
        BOOST_CHECK(false);
    });

    model.after_removed().connect([](const auto & parent, auto first, auto last) {
        BOOST_CHECK(false);
    });

    int n_after_changed = 0;
    model.after_changed().connect([this, &n_after_changed](const auto & row) {
        ++n_after_changed;
        BOOST_REQUIRE(row.is_valid());
        BOOST_CHECK_EQUAL(model.index(row), 0);
    });


    model.on_breakpoint_updated(&bp);

    BOOST_CHECK(model.childs_size(root_index) == 2);

    BOOST_CHECK(model.text(root_index, 0) == L"mock_bp_name");
    BOOST_CHECK(model.text(root_index, 1) == L"true");
    BOOST_CHECK(model.text(root_index, 2) == L"Break always (currently 1)");
    BOOST_CHECK(model.text(root_index, 3) == L"");
    BOOST_CHECK(model.text(root_index, 4) == L"");

    // check all children

    auto child1_index = model.child(root_index, 0);
    auto child2_index = model.child(root_index, 1);

    BOOST_CHECK(model.text(child1_index, 0) == L"func");
    BOOST_CHECK(model.text(child1_index, 1) == L"");
    BOOST_CHECK(model.text(child1_index, 2) == L"");
    BOOST_CHECK(model.text(child1_index, 3) == L"func");
    BOOST_CHECK(model.text(child1_index, 4) == L"0x0000000A");
    BOOST_CHECK(model.childs_size(child1_index) == 0);

    BOOST_CHECK(model.text(child2_index, 0) == L"func2");
    BOOST_CHECK(model.text(child2_index, 1) == L"");
    BOOST_CHECK(model.text(child2_index, 2) == L"");
    BOOST_CHECK(model.text(child2_index, 3) == L"func2");
    BOOST_CHECK(model.text(child2_index, 4) == L"0x0000001E");
    BOOST_CHECK(model.childs_size(child2_index) == 0);

    BOOST_CHECK_EQUAL(n_before_added, 1);
    BOOST_CHECK_EQUAL(n_after_added, 1);
    BOOST_CHECK_EQUAL(n_after_changed, 1);
}

/// Tests updating breakpoint with childs
BOOST_AUTO_TEST_CASE(update_breakpoint_childs) {
    mock_breakpoint bp(src_mdl, 10);
    bp.set_enabled(true);
    bp.set_condition("false");

    // add location
    code_position cpos{10, 20, "func", source_position{}};
    std::shared_ptr<breakpoint_location> loc{new breakpoint_location{&bp, 11, cpos}};
    bp.add_location(loc);

    // add location
    code_position cpos_x2{11, 21, "xfunc2", source_position{}};
    std::shared_ptr<breakpoint_location> loc_x2{new breakpoint_location{&bp, 100, cpos_x2}};
    bp.add_location(loc_x2);

    model.on_breakpoint_added(&bp);

    BOOST_REQUIRE_EQUAL(model.childs_size(nullindex), 1);
    auto root_index = model.child(nullindex, 0);
    BOOST_CHECK(model.text(root_index, 0) == L"mock_bp_name");
    BOOST_CHECK(model.text(root_index, 1) == L"false");
    BOOST_CHECK(model.text(root_index, 2) == L"Break always (currently 0)");
    BOOST_CHECK(model.text(root_index, 3) == L"");
    BOOST_CHECK(model.text(root_index, 4) == L"");

    BOOST_CHECK(root_index.is_valid());
    auto * bp_node = breakpoints_view_model::make_tree_node(root_index);
    BOOST_CHECK(bp_node->parent() == nullptr);

    BOOST_REQUIRE_EQUAL(model.childs_size(root_index), 2);

    bp.set_enabled(false);
    bp.set_condition("true");
    {
        code_breakpoint_info hit_info{10};
        hit_info.set_curr_hit_count(1);
        bp.breakpoint::update(&hit_info);
    }

    code_position cpos2{30, 50, "func2", source_position{}};
    std::shared_ptr<breakpoint_location> loc2{new breakpoint_location{&bp, 12, cpos2}};
    bp.add_location(loc2);

    int n_before_added = 0;
    model.before_added().connect([this, &n_before_added](const auto & parent, auto first, auto last) {
        ++n_before_added;
        BOOST_REQUIRE(parent.is_valid());
        BOOST_CHECK_EQUAL(model.index(parent), 0);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 2);
    });

    int n_after_added = 0;
    model.after_added().connect([this, &n_after_added](const auto & parent, auto first, auto last) {
        ++n_after_added;
        BOOST_REQUIRE(parent.is_valid());
        BOOST_CHECK_EQUAL(model.index(parent), 0);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 2);
    });

    int n_before_removed = 0;
    model.before_removed().connect([this, &n_before_removed](const auto & parent, auto first, auto last) {
        ++n_before_removed;
        BOOST_REQUIRE(parent.is_valid());
        BOOST_CHECK_EQUAL(model.index(parent), 0);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 1);
    });

    int n_after_removed = 0;
    model.after_removed().connect([this, &n_after_removed](const auto & parent, auto first, auto last) {
        ++n_after_removed;
        BOOST_REQUIRE(parent.is_valid());
        BOOST_CHECK_EQUAL(model.index(parent), 0);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 1);
    });

    int n_after_changed = 0;
    model.after_changed().connect([this, &n_after_changed](const auto & row) {
        ++n_after_changed;
        BOOST_REQUIRE(row.is_valid());
        BOOST_CHECK_EQUAL(model.index(row), 0);
    });


    model.on_breakpoint_updated(&bp);

    BOOST_REQUIRE_EQUAL(model.childs_size(root_index), 3);

    BOOST_CHECK(model.text(root_index, 0) == L"mock_bp_name");
    BOOST_CHECK(model.text(root_index, 1) == L"true");
    BOOST_CHECK(model.text(root_index, 2) == L"Break always (currently 1)");
    BOOST_CHECK(model.text(root_index, 3) == L"");
    BOOST_CHECK(model.text(root_index, 4) == L"");

    // check all children

    auto child1_index = model.child(root_index, 0);
    auto child2_index = model.child(root_index, 1);
    auto child3_index = model.child(root_index, 2);

    BOOST_CHECK(model.text(child1_index, 0) == L"func");
    BOOST_CHECK(model.text(child1_index, 1) == L"");
    BOOST_CHECK(model.text(child1_index, 2) == L"");
    BOOST_CHECK(model.text(child1_index, 3) == L"func");
    BOOST_CHECK(model.text(child1_index, 4) == L"0x0000000A");
    BOOST_CHECK(model.childs_size(child1_index) == 0);

    BOOST_CHECK(model.text(child2_index, 0) == L"xfunc2");
    BOOST_CHECK(model.text(child2_index, 1) == L"");
    BOOST_CHECK(model.text(child2_index, 2) == L"");
    BOOST_CHECK(model.text(child2_index, 3) == L"xfunc2");
    BOOST_CHECK(model.text(child2_index, 4) == L"0x0000000B");
    BOOST_CHECK(model.childs_size(child2_index) == 0);

    BOOST_CHECK(model.text(child3_index, 0) == L"func2");
    BOOST_CHECK(model.text(child3_index, 1) == L"");
    BOOST_CHECK(model.text(child3_index, 2) == L"");
    BOOST_CHECK(model.text(child3_index, 3) == L"func2");
    BOOST_CHECK(model.text(child3_index, 4) == L"0x0000001E");
    BOOST_CHECK(model.childs_size(child3_index) == 0);

    BOOST_CHECK_EQUAL(n_before_added, 1);
    BOOST_CHECK_EQUAL(n_after_added, 1);
    BOOST_CHECK_EQUAL(n_before_removed, 1);
    BOOST_CHECK_EQUAL(n_after_removed, 1);
    BOOST_CHECK_EQUAL(n_after_changed, 1);
}

BOOST_AUTO_TEST_CASE(remove_breakpoint) {
    // add first bp
    mock_breakpoint bp(src_mdl, 10);
    bp.set_enabled(true);
    bp.set_condition("false");
    // add location
    code_position cpos{10, 20, "func", source_position{}};
    std::shared_ptr<breakpoint_location> loc{new breakpoint_location{&bp, 11, cpos}};
    bp.add_location(loc);

    model.on_breakpoint_added(&bp);

    // add second bp
    mock_breakpoint bp2(src_mdl, 12);
    bp2.set_enabled(true);
    bp2.set_condition("1");
    // add location
    code_position cpos2{30, 50, "func2", source_position{}};
    std::shared_ptr<breakpoint_location> loc2{new breakpoint_location{&bp2, 13, cpos2}};
    bp2.add_location(loc2);

    model.on_breakpoint_added(&bp2);

    BOOST_CHECK(model.childs_size(nullindex) == 2); // 2 root nodes

    auto bp1_index = model.child(nullindex, 0);
    auto bp2_index = model.child(nullindex, 1);

    BOOST_REQUIRE(bp1_index.is_valid());
    BOOST_REQUIRE(bp2_index.is_valid());

    // recheck bp1 (root node 1)
    BOOST_CHECK(model.text(bp1_index, 0) == L"mock_bp_name (func)");
    BOOST_CHECK(model.text(bp1_index, 1) == L"false");
    BOOST_CHECK(model.text(bp1_index, 2) == L"Break always (currently 0)");
    BOOST_CHECK(model.text(bp1_index, 3) == L"func");
    BOOST_CHECK(model.text(bp1_index, 4) == L"0x0000000A");

    // check bp2 (root node 2)
    BOOST_CHECK(model.text(bp2_index, 0) == L"mock_bp_name (func2)");
    BOOST_CHECK(model.text(bp2_index, 1) == L"1");
    BOOST_CHECK(model.text(bp2_index, 2) == L"Break always (currently 0)");
    BOOST_CHECK(model.text(bp2_index, 3) == L"func2");
    BOOST_CHECK(model.text(bp2_index, 4) == L"0x0000001E");


    BOOST_CHECK(model.childs_size(bp1_index) == 0);    // one location means no children
    BOOST_CHECK(model.childs_size(bp2_index) == 0);    // one location means no children

    model.before_added().connect([this](const auto & parent, auto first, auto last) {
        BOOST_CHECK(false);
    });

    model.after_added().connect([this](const auto & parent, auto first, auto last) {
        BOOST_CHECK(false);
    });

    int n_before_removed = 0;
    model.before_removed().connect([this, &n_before_removed](const auto & parent, auto first, auto last) {
        ++n_before_removed;
        BOOST_CHECK(!parent.is_valid());
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 0);
    });

    int n_after_removed = 0;
    model.after_removed().connect([this, &n_after_removed](const auto & parent, auto first, auto last) {
        ++n_after_removed;
        BOOST_CHECK(!parent.is_valid());
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 0);
    });

    model.after_changed().connect([this](const auto & row) {
        BOOST_CHECK(false);
    });

    // now remove first bp
    model.on_breakpoint_removed(&bp);

    BOOST_CHECK(model.childs_size(nullindex) == 1); // 1 root node left

    auto bp_index = model.child(nullindex, 0);  // update bp1 index
    BOOST_REQUIRE(bp_index.is_valid());
    BOOST_CHECK(model.text(bp_index, 0) == L"mock_bp_name (func2)");
    BOOST_CHECK(model.text(bp_index, 1) == L"1");
    BOOST_CHECK(model.text(bp_index, 2) == L"Break always (currently 0)");
    BOOST_CHECK(model.text(bp_index, 3) == L"func2");
    BOOST_CHECK(model.text(bp_index, 4) == L"0x0000001E");

    BOOST_CHECK(model.childs_size(bp_index) == 0);    // one location means no children

    BOOST_CHECK_EQUAL(n_before_removed, 1);
    BOOST_CHECK_EQUAL(n_after_removed, 1);
}


BOOST_AUTO_TEST_CASE(check_watchpoints_view) {
    variable_watchpoint wp{10, true, true, 0, "var1"};
    wp.set_enabled(true);
    wp.set_condition("true");
    wp.set_address(123456);

    model.on_breakpoint_added(&wp);

    BOOST_CHECK(model.childs_size(nullindex) == 1);
    auto bp1_index = model.child(nullindex, 0);
    BOOST_REQUIRE(bp1_index.is_valid());

    // check bp1 (root node 1)
    BOOST_CHECK(model.text(bp1_index, 0) == L"When <var1> read/write");
    BOOST_CHECK(model.text(bp1_index, 1) == L"true");
    BOOST_CHECK(model.text(bp1_index, 2) == L"Break always (currently 0)");
    BOOST_CHECK(model.text(bp1_index, 3) == L"");
    BOOST_CHECK(model.text(bp1_index, 4) == L"0x0001E240");

    expression_watchpoint wp1{20, false, true, 0, "*abcdef"};
    wp1.set_enabled(true);
    wp1.set_condition("false");
    wp1.set_address(456789);

    model.on_breakpoint_added(&wp1);

    BOOST_REQUIRE(model.childs_size(nullindex) == 2);
    bp1_index = model.child(nullindex, 0);
    auto bp2_index = model.child(nullindex, 1);
    BOOST_REQUIRE(bp1_index.is_valid());
    BOOST_REQUIRE(bp2_index.is_valid());

    BOOST_CHECK(model.text(bp1_index, 0) == L"When <var1> read/write");
    BOOST_CHECK(model.text(bp1_index, 1) == L"true");
    BOOST_CHECK(model.text(bp1_index, 2) == L"Break always (currently 0)");
    BOOST_CHECK(model.text(bp1_index, 3) == L"");
    BOOST_CHECK(model.text(bp1_index, 4) == L"0x0001E240");


    BOOST_CHECK(model.text(bp2_index, 0) == L"When <*abcdef> write");
    BOOST_CHECK(model.text(bp2_index, 1) == L"false");
    BOOST_CHECK(model.text(bp2_index, 2) == L"Break always (currently 0)");
    BOOST_CHECK(model.text(bp2_index, 3) == L"");
    BOOST_CHECK(model.text(bp2_index, 4) == L"0x0006F855");
}

BOOST_AUTO_TEST_SUITE_END()

}
