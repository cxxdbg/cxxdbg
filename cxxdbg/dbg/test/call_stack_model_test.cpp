// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file call_stack_model_test.cpp
/// Contains unit tests for the call_stack_model class.

#include "cxxdbg/dbg/call_stack_model.hpp"
#include "cxxdbg/dbg/source_model.hpp"
#include "cxxdbg/dbg/stack_frame.hpp"
#include "cxxdbg/dbg/thread.hpp"
#include "cxxdbg/dbg/thread_info.hpp"
#include <boost/test/unit_test.hpp>
#include <iostream>


namespace cxxdbg::dbg::test {


struct call_stack_model_test_fixture {
    source_model sources;
    thread thrd{sources, 100};
    call_stack_model model{&thrd};
};


BOOST_FIXTURE_TEST_SUITE(call_stack_model_test, call_stack_model_test_fixture)


/// Tests call stack model initialization
BOOST_AUTO_TEST_CASE(test_init) {
    BOOST_CHECK(model.columns_size() == 3);
    BOOST_CHECK(model.column_name(0) == L"");
    BOOST_CHECK(model.column_name(1) == L"N");
    BOOST_CHECK(model.column_name(2) == L"Location");
    BOOST_CHECK(model.childs_size({}) == 0);
}


/// Tests adding stack frames into thread
BOOST_AUTO_TEST_CASE(test_add) {

    unsigned int sig_num = 0;

    // model.before_removed().connect([](auto && ...) {
    //     BOOST_CHECK(false);
    // });

    // model.after_removed().connect([](auto && ...) {
    //     BOOST_CHECK(false);
    // });

    // model.before_added().connect([this, &sig_num](auto parent, auto first, auto last) {
    //     BOOST_CHECK(sig_num == 0);
    //     ++sig_num;

    //     BOOST_CHECK_EQUAL(first, 0);
    //     BOOST_CHECK_EQUAL(last, 1);
    //     BOOST_REQUIRE_EQUAL(model.childs_size({}), 0);
    // });

    // model.after_added().connect([this, &sig_num](auto parent, auto first, auto last) {
    //     BOOST_CHECK(sig_num == 1);
    //     ++sig_num;

    //     BOOST_CHECK(first == 0);
    //     BOOST_CHECK(last == 1);

    //     BOOST_REQUIRE(model.childs_size({}) == 2);

    //     auto row1 = model.child({}, 0);
    //     BOOST_REQUIRE(row1);
    //     BOOST_CHECK(model.text(row1, 0) == L"");
    //     BOOST_CHECK(model.text(row1, 1) == L"1");
    //     BOOST_CHECK(model.text(row1, 2) == L"foo");
    //     BOOST_CHECK(model.image(row1, 0) == call_stack_model::call_stack_image_null_frame);

    //     auto row2 = model.child({}, 1);
    //     BOOST_REQUIRE(row2);
    //     BOOST_CHECK(model.text(row2, 0) == L"");
    //     BOOST_CHECK(model.text(row2, 1) == L"2");
    //     BOOST_CHECK(model.text(row2, 2) == L"bar");
    //     BOOST_CHECK(model.image(row2, 0) == call_stack_model::call_stack_image_none);
    // });

    // model.after_changed().connect([](auto && ...) {
    //     BOOST_CHECK(false);
    // });


    thread_info tinfo{100};
    tinfo.add_stack_frame({{100, 100, "foo"}, 1000, 0});
    tinfo.add_stack_frame({{200, 200, "bar"}, 2000, 0});
    thrd.update(tinfo);

//    BOOST_CHECK_EQUAL(sig_num, 2);

    BOOST_REQUIRE_EQUAL(model.childs_size({}), 2);

    auto row1 = model.child({}, 0);
    BOOST_REQUIRE(row1);
    BOOST_CHECK(model.text(row1, 0) == L"");
    BOOST_CHECK(model.text(row1, 1) == L"1");
    BOOST_CHECK(model.text(row1, 2) == L"foo");
    BOOST_CHECK(model.image(row1, 0) == call_stack_model::call_stack_image_null_frame);

    auto row2 = model.child({}, 1);
    BOOST_REQUIRE(row2);
    BOOST_CHECK(model.text(row2, 0) == L"");
    BOOST_CHECK(model.text(row2, 1) == L"2");
    BOOST_CHECK(model.text(row2, 2) == L"bar");
    BOOST_CHECK(model.image(row2, 0) == call_stack_model::call_stack_image_none);
}


/// Tests removing stack frames from thread
BOOST_AUTO_TEST_CASE(test_remove) {

    {
        thread_info tinfo{100};
        tinfo.add_stack_frame({{100, 100, "foo"}, 1000, 0});
        tinfo.add_stack_frame({{200, 200, "bar"}, 2000, 0});
        thrd.update(tinfo);
    }

    // unsigned int sig_num = 0;

    // model.before_removed().connect([this, &sig_num](auto parent, auto first, auto last) {
    //     BOOST_CHECK(sig_num == 0);
    //     ++sig_num;

    //     BOOST_CHECK(first == 0);
    //     BOOST_CHECK(last == 0);

    //     BOOST_REQUIRE(model.childs_size({}) == 2);

    //     auto row1 = model.child({}, 0);
    //     BOOST_REQUIRE(row1);
    //     BOOST_CHECK(model.text(row1, 0) == L"");
    //     BOOST_CHECK(model.text(row1, 1) == L"1");
    //     BOOST_CHECK(model.text(row1, 2) == L"foo");
    //     BOOST_CHECK(model.image(row1, 0) == call_stack_model::call_stack_image_null_frame);

    //     auto row2 = model.child({}, 1);
    //     BOOST_REQUIRE(row2);
    //     BOOST_CHECK(model.text(row2, 0) == L"");
    //     BOOST_CHECK(model.text(row2, 1) == L"2");
    //     BOOST_CHECK(model.text(row2, 2) == L"bar");
    //     BOOST_CHECK(model.image(row2, 0) == call_stack_model::call_stack_image_none);
    // });

    // model.after_removed().connect([this, &sig_num](auto parent, auto first, auto last) {
    //     BOOST_CHECK(sig_num == 1);
    //     ++sig_num;

    //     BOOST_CHECK(first == 0);
    //     BOOST_CHECK(last == 0);

    //     BOOST_REQUIRE(model.childs_size({}) == 1);

    //     auto row1 = model.child({}, 0);
    //     BOOST_REQUIRE(row1);
    //     BOOST_CHECK(model.text(row1, 0) == L"");
    //     BOOST_CHECK(model.text(row1, 1) == L"1");
    //     BOOST_CHECK(model.text(row1, 2) == L"bar");
    //     BOOST_CHECK(model.image(row1, 0) == call_stack_model::call_stack_image_null_frame);
    // });

    // model.before_added().connect([](auto && ...) {
    //     BOOST_CHECK(false);
    // });

    // model.after_added().connect([](auto && ...) {
    //     BOOST_CHECK(false);
    // });

    // model.after_changed().connect([this, &sig_num](const auto & row) {
    //     BOOST_CHECK(sig_num == 2);
    //     ++sig_num;
    //     BOOST_REQUIRE(row);
    //     BOOST_CHECK(model.index(row) == 0);

    //     BOOST_REQUIRE(model.childs_size({}) == 1);

    //     auto row1 = model.child({}, 0);
    //     BOOST_REQUIRE(row1);
    //     BOOST_CHECK(model.text(row1, 0) == L"");
    //     BOOST_CHECK(model.text(row1, 1) == L"1");
    //     BOOST_CHECK(model.text(row1, 2) == L"bar");
    //     BOOST_CHECK(model.image(row1, 0) == call_stack_model::call_stack_image_null_frame);
    // });


    {
        thread_info tinfo{100};
        tinfo.add_stack_frame({{200, 200, "bar"}, 2000, 0});
        thrd.update(tinfo);
    }

    // BOOST_CHECK(sig_num == 3);

    BOOST_REQUIRE(model.childs_size({}) == 1);

    auto row1 = model.child({}, 0);
    BOOST_REQUIRE(row1);
    BOOST_CHECK(model.text(row1, 0) == L"");
    BOOST_CHECK(model.text(row1, 1) == L"1");
    BOOST_CHECK(model.text(row1, 2) == L"bar");
    BOOST_CHECK(model.image(row1, 0) == call_stack_model::call_stack_image_null_frame);
}


/// Tests updating stack frame in thread
BOOST_AUTO_TEST_CASE(test_update) {

    {
        thread_info tinfo{100};
        tinfo.add_stack_frame({{100, 100, "foo"}, 1000, 0});
        thrd.update(tinfo);
    }

    unsigned int sig_num = 0;

    model.before_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.after_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.before_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.after_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.after_changed().connect([this, &sig_num](const auto & idx) {
        BOOST_CHECK(sig_num == 0);
        ++sig_num;

        BOOST_REQUIRE(idx.is_valid());
        BOOST_CHECK(model.text(idx, 0) == L"");
        BOOST_CHECK(model.text(idx, 1) == L"1");
        BOOST_CHECK(model.text(idx, 2) == L"foo");
        BOOST_CHECK(model.image(idx, 0) == call_stack_model::call_stack_image_null_frame);
    });


    {
        thread_info tinfo{100};
        stack_frame_info finfo{{100, 100, "foo"}, 1000, 0};
        finfo.add_param({L"x", L"int", L"10"});
        tinfo.add_stack_frame(finfo);
        thrd.update(tinfo);
    }

    BOOST_CHECK(sig_num == 1);
    BOOST_REQUIRE(model.childs_size({}) == 1);

    auto idx = model.child({}, 0);
    BOOST_REQUIRE(idx);

    BOOST_CHECK(model.text(idx, 0) == L"");
    BOOST_CHECK(model.text(idx, 1) == L"1");
    BOOST_CHECK(model.text(idx, 2) == L"foo");
    BOOST_CHECK(model.image(idx, 0) == call_stack_model::call_stack_image_null_frame);
}


/// Tests setting current stack frame in thread
BOOST_AUTO_TEST_CASE(test_set_current) {

    {
        thread_info tinfo{100};
        tinfo.add_stack_frame({{100, 100, "foo"}, 1000, 0});
        tinfo.add_stack_frame({{200, 200, "bar"}, 2000, 0});
        thrd.update(tinfo);
        thrd.set_current_frame_index(0);
    }

    unsigned int sig_num = 0;

    model.before_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.after_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.before_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.after_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.after_changed().connect([this, &sig_num](const auto & idx) {
        if (sig_num == 0) {
            BOOST_CHECK(model.index(idx) == 0);
        } else if (sig_num == 1) {
            BOOST_CHECK(model.index(idx) == 1);
        } else {
            BOOST_CHECK(false);
        }

        BOOST_REQUIRE(model.childs_size({}) == 2);

        auto idx1 = model.child({}, 0);
        BOOST_REQUIRE(idx1);
        BOOST_CHECK(model.text(idx1, 0) == L"");
        BOOST_CHECK(model.text(idx1, 1) == L"1");
        BOOST_CHECK(model.text(idx1, 2) == L"foo");
        BOOST_CHECK(model.image(idx1, 0) == call_stack_model::call_stack_image_null_frame);

        auto idx2 = model.child({}, 1);
        BOOST_REQUIRE(idx2);
        BOOST_CHECK(model.text(idx2, 0) == L"");
        BOOST_CHECK(model.text(idx2, 1) == L"2");
        BOOST_CHECK(model.text(idx2, 2) == L"bar");
        BOOST_CHECK(model.image(idx2, 0) == call_stack_model::call_stack_image_current_frame);

        ++sig_num;
    });

    thrd.set_current_frame_index(1);

    BOOST_CHECK(sig_num == 2);

    BOOST_REQUIRE(model.childs_size({}) == 2);

    auto idx1 = model.child({}, 0);
    BOOST_REQUIRE(idx1);
    BOOST_CHECK(model.text(idx1, 0) == L"");
    BOOST_CHECK(model.text(idx1, 1) == L"1");
    BOOST_CHECK(model.text(idx1, 2) == L"foo");
    BOOST_CHECK(model.image(idx1, 0) == call_stack_model::call_stack_image_null_frame);

    auto idx2 = model.child({}, 1);
    BOOST_REQUIRE(idx2);
    BOOST_CHECK(model.text(idx2, 0) == L"");
    BOOST_CHECK(model.text(idx2, 1) == L"2");
    BOOST_CHECK(model.text(idx2, 2) == L"bar");
    BOOST_CHECK(model.image(idx2, 0) == call_stack_model::call_stack_image_current_frame);
}


/// Tests changing display param types flag
BOOST_AUTO_TEST_CASE(change_par_types) {
    {
        thread_info tinfo{100};
        stack_frame_info finfo{{100, 100, "foo"}, 1000, 0};
        finfo.add_param({L"x", L"int", L"10"});
        tinfo.add_stack_frame(finfo);
        thrd.update(tinfo);
        thrd.set_current_frame_index(0);
    }

    {
        BOOST_REQUIRE(model.childs_size({}) == 1);
        auto child = model.child({}, 0);
        BOOST_REQUIRE(child);
        BOOST_CHECK(model.text(child, 2) == L"foo");
    }


    model.before_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.after_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.before_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.after_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    unsigned int sig_num = 0;

    model.after_changed().connect([this, &sig_num](const auto & idx) {
        BOOST_CHECK(sig_num == 0);
        ++sig_num;

        BOOST_REQUIRE(idx);
        BOOST_CHECK(model.index(idx) == 0);
        BOOST_REQUIRE(model.childs_size({}) == 1);
        auto child = model.child({}, 0);
        BOOST_REQUIRE(child);
        BOOST_CHECK(model.text(child, 2) == L"foo(int)");
    });

    model.set_disp_par_types(true);

    BOOST_CHECK(sig_num == 1);

    BOOST_REQUIRE(model.childs_size({}) == 1);
    auto child = model.child({}, 0);
    BOOST_REQUIRE(child);
    BOOST_CHECK(model.text(child, 2) == L"foo(int)");
}


/// Tests changing display param names flag
BOOST_AUTO_TEST_CASE(change_par_names) {
    {
        thread_info tinfo{100};
        stack_frame_info finfo{{100, 100, "foo"}, 1000, 0};
        finfo.add_param({L"x", L"int", L"10"});
        tinfo.add_stack_frame(finfo);
        thrd.update(tinfo);
        thrd.set_current_frame_index(0);
    }

    {
        BOOST_REQUIRE(model.childs_size({}) == 1);
        auto child = model.child({}, 0);
        BOOST_REQUIRE(child);
        BOOST_CHECK(model.text(child, 2) == L"foo");
    }


    model.before_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.after_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.before_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.after_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    unsigned int sig_num = 0;

    model.after_changed().connect([this, &sig_num](const auto & idx) {
        BOOST_CHECK(sig_num == 0);
        ++sig_num;

        BOOST_REQUIRE(idx);
        BOOST_CHECK(model.index(idx) == 0);
        BOOST_REQUIRE(model.childs_size({}) == 1);
        auto child = model.child({}, 0);
        BOOST_REQUIRE(child);
        BOOST_CHECK(model.text(child, 2) == L"foo(x)");
    });

    model.set_disp_par_names(true);

    BOOST_CHECK(sig_num == 1);

    BOOST_REQUIRE(model.childs_size({}) == 1);
    auto child = model.child({}, 0);
    BOOST_REQUIRE(child);
    BOOST_CHECK(model.text(child, 2) == L"foo(x)");
}


/// Tests changing display param values flag
BOOST_AUTO_TEST_CASE(change_par_vals) {
    {
        thread_info tinfo{100};
        stack_frame_info finfo{{100, 100, "foo"}, 1000, 0};
        finfo.add_param({L"x", L"int", L"10"});
        tinfo.add_stack_frame(finfo);
        thrd.update(tinfo);
        thrd.set_current_frame_index(0);
    }

    {
        BOOST_REQUIRE(model.childs_size({}) == 1);
        auto child = model.child({}, 0);
        BOOST_REQUIRE(child);
        BOOST_CHECK(model.text(child, 2) == L"foo");
    }


    model.before_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.after_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.before_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.after_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    unsigned int sig_num = 0;

    model.after_changed().connect([this, &sig_num](const auto & idx) {
        BOOST_CHECK(sig_num == 0);
        ++sig_num;

        BOOST_REQUIRE(idx);
        BOOST_CHECK(model.index(idx) == 0);
        BOOST_REQUIRE(model.childs_size({}) == 1);
        auto child = model.child({}, 0);
        BOOST_REQUIRE(child);
        BOOST_CHECK(model.text(child, 2) == L"foo(10)");
    });

    model.set_disp_par_vals(true);

    BOOST_CHECK(sig_num == 1);

    BOOST_REQUIRE(model.childs_size({}) == 1);
    auto child = model.child({}, 0);
    BOOST_REQUIRE(child);
    BOOST_CHECK(model.text(child, 2) == L"foo(10)");
}


/// Tests setting thread in call stack model
BOOST_AUTO_TEST_CASE(test_set_thread) {
    thread thrd2{sources, 200};

    {
        thread_info tinfo{100};
        tinfo.add_stack_frame({{100, 100, "foo"}, 1000, 0});
        thrd.update(tinfo);
    }

    {
        thread_info tinfo{200};
        tinfo.add_stack_frame({{200, 200, "bar"}, 2000, 0});
        thrd2.update(tinfo);
    }

    // unsigned int sig_num = 0;

    // model.before_removed().connect([this, &sig_num](const auto & row, auto first, auto last) {
    //     BOOST_CHECK(sig_num == 0);
    //     ++sig_num;

    //     BOOST_CHECK(!row);
    //     BOOST_CHECK(first == 0);
    //     BOOST_CHECK(last == 0);

    //     BOOST_REQUIRE(model.childs_size({}) == 1);
    //     auto child = model.child({}, 0);
    //     BOOST_REQUIRE(child);
    //     BOOST_CHECK(model.text(child, 0) == L"");
    //     BOOST_CHECK(model.text(child, 1) == L"1");
    //     BOOST_CHECK(model.text(child, 2) == L"foo");
    // });

    // model.after_removed().connect([this, &sig_num](const auto & row, auto first, auto last) {
    //     BOOST_CHECK(sig_num == 1);
    //     ++sig_num;

    //     BOOST_CHECK(!row);
    //     BOOST_CHECK(first == 0);
    //     BOOST_CHECK(last == 0);

    //     BOOST_REQUIRE_EQUAL(model.childs_size({}), 0);
    // });

    // model.before_added().connect([this, &sig_num](const auto & row, auto first, auto last) {
    //     BOOST_CHECK(sig_num == 2);
    //     ++sig_num;

    //     BOOST_CHECK(!row);
    //     BOOST_CHECK(first == 0);
    //     BOOST_CHECK(last == 0);

    //     BOOST_REQUIRE(model.childs_size({}) == 0);
    // });

    // model.after_added().connect([this, &sig_num](const auto & row, auto first, auto last) {
    //     BOOST_CHECK(sig_num == 3);
    //     ++sig_num;

    //     BOOST_CHECK(!row);
    //     BOOST_CHECK(first == 0);
    //     BOOST_CHECK(last == 0);

    //     BOOST_REQUIRE(model.childs_size({}) == 1);
    //     auto child = model.child({}, 0);
    //     BOOST_REQUIRE(child);
    //     BOOST_CHECK(model.text(child, 0) == L"");
    //     BOOST_CHECK(model.text(child, 1) == L"1");
    //     BOOST_CHECK(model.text(child, 2) == L"bar");
    // });

    // model.after_changed().connect([](auto && ...) {
    //     BOOST_CHECK(false);
    // });

    model.set_thread(&thrd2);

    // BOOST_CHECK(sig_num == 4);

    BOOST_REQUIRE(model.childs_size({}) == 1);
    auto child = model.child({}, 0);
    BOOST_REQUIRE(child);
    BOOST_CHECK(model.text(child, 0) == L"");
    BOOST_CHECK(model.text(child, 1) == L"1");
    BOOST_CHECK(model.text(child, 2) == L"bar");
}


/// Tests setting same thread in call stack model
BOOST_AUTO_TEST_CASE(test_set_thread_same) {
    {
        thread_info tinfo{100};
        tinfo.add_stack_frame({{100, 100, "foo"}, 1000, 0});
        thrd.update(tinfo);
    }

    model.before_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.after_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.before_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.after_added().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.after_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.set_thread(&thrd);

    BOOST_REQUIRE(model.childs_size({}) == 1);
    auto child = model.child({}, 0);
    BOOST_REQUIRE(child);
    BOOST_CHECK(model.text(child, 0) == L"");
    BOOST_CHECK(model.text(child, 1) == L"1");
    BOOST_CHECK(model.text(child, 2) == L"foo");
}


/// Tests displaying incomplete call stack
BOOST_AUTO_TEST_CASE(test_incomplete) {
    {
        thread_info tinfo{100};
        tinfo.add_stack_frame({{400, 400, "foo"}, 4000, 0});
        tinfo.set_has_complete_call_stack(false);
        thrd.update(tinfo);
    }

    BOOST_REQUIRE(model.childs_size({}) == 2);

    auto child1 = model.child({}, 0);
    BOOST_REQUIRE(child1);
    BOOST_CHECK(model.text(child1, 0) == L"");
    BOOST_CHECK(model.text(child1, 1) == L"1");
    BOOST_CHECK(model.text(child1, 2) == L"foo");

    auto child2 = model.child({}, 1);
    BOOST_REQUIRE(child2);
    BOOST_CHECK(model.text(child2, 0) == L"");
    BOOST_CHECK(model.text(child2, 1) == L"...");
    BOOST_CHECK(model.text(child2, 2) == L"...");
}


/// Tests adding ... item
BOOST_AUTO_TEST_CASE(test_add_incomplete_item) {
    {
        thread_info tinfo{100};
        tinfo.add_stack_frame({{400, 400, "foo"}, 4000, 0});
        thrd.update(tinfo);
    }

    unsigned int sig_num = 0;

    model.before_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.after_removed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    model.before_added().connect([this, &sig_num](const auto & row, auto first, auto last) {
        if (sig_num == 0) {
            // adding ...

            BOOST_CHECK(!row);
            BOOST_CHECK(first == 1);
            BOOST_CHECK(last == 1);

            BOOST_REQUIRE(model.childs_size({}) == 1);

            auto child1 = model.child({}, 0);
            BOOST_REQUIRE(child1);

            BOOST_CHECK(model.text(child1, 0) == L"");
            BOOST_CHECK(model.text(child1, 1) == L"1");
            BOOST_CHECK(model.text(child1, 2) == L"foo");

        } else if (sig_num == 2) {
            // adding item

            BOOST_CHECK(!row);
            BOOST_CHECK_EQUAL(first, 1);
            BOOST_CHECK_EQUAL(last, 1);

            BOOST_REQUIRE(model.childs_size({}) == 2);

            auto child1 = model.child({}, 0);
            BOOST_REQUIRE(child1);
            BOOST_CHECK(model.text(child1, 0) == L"");
            BOOST_CHECK(model.text(child1, 1) == L"1");
            BOOST_CHECK(model.text(child1, 2) == L"foo");

            auto child2 = model.child({}, 1);
            BOOST_REQUIRE(child2);
            BOOST_CHECK(model.text(child2, 0) == L"");
            BOOST_CHECK(model.text(child2, 1) == L"...");
        } else {
            BOOST_CHECK(false);
        }

        ++sig_num;
    });

    model.after_added().connect([this, &sig_num](const auto & row, auto first, auto last) {
        if (sig_num == 1) {
            // adding ...

            BOOST_CHECK(!row);
            BOOST_CHECK(first == 1);
            BOOST_CHECK(last == 1);

            BOOST_REQUIRE(model.childs_size({}) == 2);

            auto child1 = model.child({}, 0);
            BOOST_REQUIRE(child1);
            BOOST_CHECK(model.text(child1, 0) == L"");
            BOOST_CHECK(model.text(child1, 1) == L"1");
            BOOST_CHECK(model.text(child1, 2) == L"foo");

            auto child2 = model.child({}, 1);
            BOOST_REQUIRE(child2);
            BOOST_CHECK(model.text(child2, 0) == L"");
            BOOST_CHECK(model.text(child2, 1) == L"...");
            BOOST_CHECK(model.text(child2, 2) == L"...");

        } else if (sig_num == 3) {
            // adding item

            BOOST_CHECK(!row);
            BOOST_CHECK(first == 1);
            BOOST_CHECK(last == 1);

            BOOST_REQUIRE(model.childs_size({}) == 3);

            auto child1 = model.child({}, 0);
            BOOST_REQUIRE(child1);
            BOOST_CHECK(model.text(child1, 0) == L"");
            BOOST_CHECK(model.text(child1, 1) == L"1");
            BOOST_CHECK(model.text(child1, 2) == L"foo");

            auto child2 = model.child({}, 1);
            BOOST_REQUIRE(child2);
            BOOST_CHECK(model.text(child2, 0) == L"");
            BOOST_CHECK(model.text(child2, 1) == L"2");
            BOOST_CHECK(model.text(child2, 2) == L"bar");

            auto child3 = model.child({}, 2);
            BOOST_REQUIRE(child3);
            BOOST_CHECK(model.text(child3, 0) == L"");
            BOOST_CHECK(model.text(child3, 1) == L"...");
            BOOST_CHECK(model.text(child3, 2) == L"...");
        } else {
            BOOST_CHECK(false);
        }

        ++sig_num;
    });

    model.after_changed().connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    {
        thread_info tinfo{100};
        tinfo.add_stack_frame({{400, 400, "foo"}, 4000, 0});
        tinfo.add_stack_frame({{500, 500, "bar"}, 5000, 0});
        tinfo.set_has_complete_call_stack(false);
        thrd.update(tinfo);
    }

    BOOST_CHECK(sig_num == 4);

    BOOST_REQUIRE(model.childs_size({}) == 3);

    auto child1 = model.child({}, 0);
    BOOST_REQUIRE(child1);
    BOOST_CHECK(model.text(child1, 0) == L"");
    BOOST_CHECK(model.text(child1, 1) == L"1");
    BOOST_CHECK(model.text(child1, 2) == L"foo");

    auto child2 = model.child({}, 1);
    BOOST_REQUIRE(child2);
    BOOST_CHECK(model.text(child2, 0) == L"");
    BOOST_CHECK(model.text(child2, 1) == L"2");
    BOOST_CHECK(model.text(child2, 2) == L"bar");

    auto child3 = model.child({}, 2);
    BOOST_REQUIRE(child3);
    BOOST_CHECK(model.text(child3, 0) == L"");
    BOOST_CHECK(model.text(child3, 1) == L"...");
    BOOST_CHECK(model.text(child3, 2) == L"...");
}


/// Tests removing ... item
BOOST_AUTO_TEST_CASE(test_remove_incomplete_item) {
    {
        thread_info tinfo{100};
        tinfo.add_stack_frame({{400, 400, "foo"}, 4000, 0});
        tinfo.add_stack_frame({{500, 500, "bar"}, 5000, 0});
        tinfo.set_has_complete_call_stack(false);
        thrd.update(tinfo);
    }

    // unsigned int sig_num = 0;

    // model.before_removed().connect([this, &sig_num](const auto & row, auto first, auto last) {

    //     if (sig_num == 0) {
    //         // removing ...

    //         BOOST_CHECK(!row);
    //         BOOST_CHECK(first == 2);
    //         BOOST_CHECK(last == 2);

    //         BOOST_REQUIRE(model.childs_size({}) == 3);

    //         auto child1 = model.child({}, 0);
    //         BOOST_REQUIRE(child1);
    //         BOOST_CHECK(model.text(child1, 0) == L"");
    //         BOOST_CHECK(model.text(child1, 1) == L"1");
    //         BOOST_CHECK(model.text(child1, 2) == L"foo");

    //         auto child2 = model.child({}, 1);
    //         BOOST_REQUIRE(child2);
    //         BOOST_CHECK(model.text(child2, 0) == L"");
    //         BOOST_CHECK(model.text(child2, 1) == L"2");
    //         BOOST_CHECK(model.text(child2, 2) == L"bar");

    //         auto child3 = model.child({}, 2);
    //         BOOST_REQUIRE(child3);
    //         BOOST_CHECK(model.text(child3, 0) == L"");
    //         BOOST_CHECK(model.text(child3, 1) == L"...");
    //         BOOST_CHECK(model.text(child3, 2) == L"...");

    //     } else if (sig_num == 2) {
    //         // removing item

    //         BOOST_CHECK(!row);
    //         BOOST_CHECK(first == 1);
    //         BOOST_CHECK(last == 1);

    //         BOOST_REQUIRE(model.childs_size({}) == 2);

    //         auto child1 = model.child({}, 0);
    //         BOOST_REQUIRE(child1);
    //         BOOST_CHECK(model.text(child1, 0) == L"");
    //         BOOST_CHECK(model.text(child1, 1) == L"1");
    //         BOOST_CHECK(model.text(child1, 2) == L"foo");

    //         auto child2 = model.child({}, 1);
    //         BOOST_REQUIRE(child2);
    //         BOOST_CHECK(model.text(child2, 0) == L"");
    //         BOOST_CHECK(model.text(child2, 1) == L"2");
    //         BOOST_CHECK(model.text(child2, 2) == L"bar");

    //     } else {
    //         BOOST_CHECK(false);
    //     }

    //     ++sig_num;
    // });

    // model.after_removed().connect([this, &sig_num](const auto & row, auto first, auto last) {

    //     if (sig_num == 1) {
    //         // removing ...

    //         BOOST_CHECK(!row);
    //         BOOST_CHECK(first == 2);
    //         BOOST_CHECK(last == 2);

    //         BOOST_REQUIRE(model.childs_size({}) == 2);

    //         auto child1 = model.child({}, 0);
    //         BOOST_REQUIRE(child1);
    //         BOOST_CHECK(model.text(child1, 0) == L"");
    //         BOOST_CHECK(model.text(child1, 1) == L"1");
    //         BOOST_CHECK(model.text(child1, 2) == L"foo");

    //         auto child2 = model.child({}, 1);
    //         BOOST_REQUIRE(child2);
    //         BOOST_CHECK(model.text(child2, 0) == L"");
    //         BOOST_CHECK(model.text(child2, 1) == L"2");
    //         BOOST_CHECK(model.text(child2, 2) == L"bar");

    //     } else if (sig_num == 3) {
    //         // removing item

    //         BOOST_CHECK(!row);
    //         BOOST_CHECK(first == 1);
    //         BOOST_CHECK(last == 1);

    //         BOOST_REQUIRE(model.childs_size({}) == 1);

    //         auto child1 = model.child({}, 0);
    //         BOOST_REQUIRE(child1);
    //         BOOST_CHECK(model.text(child1, 0) == L"");
    //         BOOST_CHECK(model.text(child1, 1) == L"1");
    //         BOOST_CHECK(model.text(child1, 2) == L"foo");

    //     } else {
    //         BOOST_CHECK(false);
    //     }

    //     ++sig_num;
    // });

    // model.before_added().connect([](auto && ...) {
    //     BOOST_CHECK(false);
    // });

    // model.after_added().connect([](auto && ...) {
    //     BOOST_CHECK(false);
    // });

    // model.after_changed().connect([](auto && ...) {
    //     BOOST_CHECK(false);
    // });


    {
        thread_info tinfo{100};
        tinfo.add_stack_frame({{400, 400, "foo"}, 4000, 0});
        thrd.update(tinfo);
    }

    // BOOST_CHECK(sig_num == 4);

    BOOST_REQUIRE(model.childs_size({}) == 1);

    auto child1 = model.child({}, 0);
    BOOST_REQUIRE(child1);
    BOOST_CHECK(model.text(child1, 0) == L"");
    BOOST_CHECK(model.text(child1, 1) == L"1");
    BOOST_CHECK(model.text(child1, 2) == L"foo");
}

// no grouping when matcher is empty
BOOST_AUTO_TEST_CASE(test_group_items_no_grouping) {
    {
        model.group_functions_lists_changed("");    // empty matcher, no grouped items

        thread_info tinfo{100};
        tinfo.add_stack_frame({{400, 400, "aaa"}, 4000, 0});
        tinfo.add_stack_frame({{500, 500, "bbb"}, 5000, 0});
        tinfo.add_stack_frame({{600, 600, "ccc"}, 6000, 0});
        tinfo.add_stack_frame({{700, 700, "ddd"}, 7000, 0});

        thrd.update(tinfo);
    }
    // structure is plain after init
    BOOST_REQUIRE(model.childs_size({}) == 4);

    auto child1 = model.child({}, 0);
    BOOST_REQUIRE(child1);
    BOOST_CHECK(model.text(child1, 0) == L"");
    BOOST_CHECK(model.text(child1, 1) == L"1");
    BOOST_CHECK(model.text(child1, 2) == L"aaa");

    auto child2 = model.child({}, 1);
    BOOST_REQUIRE(child2);
    BOOST_CHECK(model.text(child2, 0) == L"");
    BOOST_CHECK(model.text(child2, 1) == L"2");
    BOOST_CHECK(model.text(child2, 2) == L"bbb");

    auto child3 = model.child({}, 2);
    BOOST_REQUIRE(child3);
    BOOST_CHECK(model.text(child3, 0) == L"");
    BOOST_CHECK(model.text(child3, 1) == L"3");
    BOOST_CHECK(model.text(child3, 2) == L"ccc");

    auto child4 = model.child({}, 3);
    BOOST_REQUIRE(child4);
    BOOST_CHECK(model.text(child4, 0) == L"");
    BOOST_CHECK(model.text(child4, 1) == L"4");
    BOOST_CHECK(model.text(child4, 2) == L"ddd");
}

// settign group regex from the beginning allows grouping items
BOOST_AUTO_TEST_CASE(test_group_items) {
    {
        model.group_functions_lists_changed("(^aaa|^bbb|^ccc)"); // ccc and ddd are grouped items
        // structure will be  aaa->(bbb, ccc), ddd

        thread_info tinfo{100};
        tinfo.add_stack_frame({{400, 400, "aaa"}, 4000, 0});
        tinfo.add_stack_frame({{500, 500, "bbb"}, 5000, 0});
        tinfo.add_stack_frame({{600, 600, "ccc"}, 6000, 0});
        tinfo.add_stack_frame({{700, 700, "ddd"}, 7000, 0});

        thrd.update(tinfo);
    }

    BOOST_REQUIRE(model.childs_size({}) == 2);

    auto child1 = model.child({}, 0);
    BOOST_REQUIRE(child1);
    BOOST_CHECK(model.text(child1, 0) == L"");
    BOOST_CHECK(model.text(child1, 1) == L"1");
    BOOST_CHECK(model.text(child1, 2) == L"aaa");

    BOOST_REQUIRE(model.childs_size(child1) == 2);

    auto child2 = model.child(child1, 0);
    BOOST_REQUIRE(child2);
    BOOST_CHECK(model.text(child2, 0) == L"");
    BOOST_CHECK(model.text(child2, 1) == L"2");
    BOOST_CHECK(model.text(child2, 2) == L"bbb");
    BOOST_CHECK(model.childs_size(child2) == 0);

    auto child3 = model.child(child1, 1);
    BOOST_REQUIRE(child3);
    BOOST_CHECK(model.text(child3, 0) == L"");
    BOOST_CHECK(model.text(child3, 1) == L"3");
    BOOST_CHECK(model.text(child3, 2) == L"ccc");
    BOOST_CHECK(model.childs_size(child3) == 0);

    auto child4 = model.child({}, 1);
    BOOST_REQUIRE(child4);
    BOOST_CHECK(model.text(child4, 0) == L"");
    BOOST_CHECK(model.text(child4, 1) == L"4");
    BOOST_CHECK(model.text(child4, 2) == L"ddd");
    BOOST_CHECK(model.childs_size(child4) == 0);
}

// when group regex changes, items must be regrouped
BOOST_AUTO_TEST_CASE(test_regroup_items) {
    {
        model.group_functions_lists_changed("");    // empty matcher, no grouped items

        thread_info tinfo{100};
        tinfo.add_stack_frame({{400, 400, "aaa"}, 4000, 0});
        tinfo.add_stack_frame({{500, 500, "bbb"}, 5000, 0});
        tinfo.add_stack_frame({{600, 600, "ccc"}, 6000, 0});
        tinfo.add_stack_frame({{700, 700, "ddd"}, 7000, 0});

        thrd.update(tinfo);
    }
    {
        // structure is plain after init
        BOOST_REQUIRE(model.childs_size({}) == 4);

        auto child1 = model.child({}, 0);
        BOOST_REQUIRE(child1);
        BOOST_REQUIRE(model.text(child1, 0) == L"");
        BOOST_REQUIRE(model.text(child1, 1) == L"1");
        BOOST_REQUIRE(model.text(child1, 2) == L"aaa");

        auto child2 = model.child({}, 1);
        BOOST_REQUIRE(child2);
        BOOST_REQUIRE(model.text(child2, 0) == L"");
        BOOST_REQUIRE(model.text(child2, 1) == L"2");
        BOOST_REQUIRE(model.text(child2, 2) == L"bbb");

        auto child3 = model.child({}, 2);
        BOOST_REQUIRE(child3);
        BOOST_REQUIRE(model.text(child3, 0) == L"");
        BOOST_REQUIRE(model.text(child3, 1) == L"3");
        BOOST_REQUIRE(model.text(child3, 2) == L"ccc");

        auto child4 = model.child({}, 3);
        BOOST_REQUIRE(child4);
        BOOST_REQUIRE(model.text(child4, 0) == L"");
        BOOST_REQUIRE(model.text(child4, 1) == L"4");
        BOOST_REQUIRE(model.text(child4, 2) == L"ddd");
    }

    {
        model.group_functions_lists_changed("(^aaa|^bbb)"); // aaa and bbb are grouped items
        // grouped item at the beginning (aaa) is root nevertheless
        // structure will be aaa->(bbb), ccc, ddd

        BOOST_REQUIRE(model.childs_size({}) == 3);

        auto child1 = model.child({}, 0);
        BOOST_REQUIRE(child1);
        BOOST_CHECK(model.text(child1, 0) == L"");
        BOOST_CHECK(model.text(child1, 1) == L"1");
        BOOST_CHECK(model.text(child1, 2) == L"aaa");

        BOOST_REQUIRE(model.childs_size(child1) == 1);

        auto child2 = model.child(child1, 0);
        BOOST_REQUIRE(child2);
        BOOST_CHECK(model.text(child2, 0) == L"");
        BOOST_CHECK(model.text(child2, 1) == L"2");
        BOOST_CHECK(model.text(child2, 2) == L"bbb");
        BOOST_CHECK(model.childs_size(child2) == 0);

        auto child3 = model.child({}, 1);
        BOOST_REQUIRE(child3);
        BOOST_CHECK(model.text(child3, 0) == L"");
        BOOST_CHECK(model.text(child3, 1) == L"3");
        BOOST_CHECK(model.text(child3, 2) == L"ccc");
        BOOST_CHECK(model.childs_size(child3) == 0);

        auto child4 = model.child({}, 2);
        BOOST_REQUIRE(child4);
        BOOST_CHECK(model.text(child4, 0) == L"");
        BOOST_CHECK(model.text(child4, 1) == L"4");
        BOOST_CHECK(model.text(child4, 2) == L"ddd");
        BOOST_CHECK(model.childs_size(child4) == 0);
    }

    {
        model.group_functions_lists_changed("(^ccc|^ddd)"); // ccc and ddd are grouped items
        // structure will be  aaa, bbb->(ccc, ddd)

        BOOST_REQUIRE_EQUAL(model.childs_size({}), 2);

        auto child1 = model.child({}, 0);
        BOOST_REQUIRE(child1);
        BOOST_CHECK(model.text(child1, 0) == L"");
        BOOST_CHECK(model.text(child1, 1) == L"1");
        BOOST_CHECK(model.text(child1, 2) == L"aaa");
        BOOST_CHECK(model.childs_size(child1) == 0);

        auto child2 = model.child({}, 1);
        BOOST_REQUIRE(child2);
        BOOST_CHECK(model.text(child2, 0) == L"");
        BOOST_CHECK(model.text(child2, 1) == L"2");
        BOOST_CHECK(model.text(child2, 2) == L"bbb");
        BOOST_CHECK(model.childs_size(child2) == 2);

        auto child3 = model.child(child2, 0);
        BOOST_REQUIRE(child3);
        BOOST_CHECK(model.text(child3, 0) == L"");
        BOOST_CHECK(model.text(child3, 1) == L"3");
        BOOST_CHECK(model.text(child3, 2) == L"ccc");
        BOOST_CHECK(model.childs_size(child3) == 0);

        auto child4 = model.child(child2, 1);
        BOOST_REQUIRE(child4);
        BOOST_CHECK(model.text(child4, 0) == L"");
        BOOST_CHECK(model.text(child4, 1) == L"4");
        BOOST_CHECK(model.text(child4, 2) == L"ddd");
        BOOST_CHECK(model.childs_size(child4) == 0);
    }
}

// last fake item is not grouped and doesn't interfere with grouped items
BOOST_AUTO_TEST_CASE(test_group_items_fake_last_item_interference) {
    {
        model.group_functions_lists_changed("(^aaa|^bbb)"); // ccc and ddd are grouped items
        // structure will be  aaa->(bbb, ccc), ddd

        thread_info tinfo{100};
        tinfo.add_stack_frame({{400, 400, "aaa"}, 4000, 0});
        tinfo.add_stack_frame({{500, 500, "bbb"}, 5000, 0});
        tinfo.set_has_complete_call_stack(false);
        thrd.update(tinfo);
    }

    BOOST_REQUIRE(model.childs_size({}) == 2);

    auto child1 = model.child({}, 0);
    BOOST_REQUIRE(child1);
    BOOST_CHECK(model.text(child1, 0) == L"");
    BOOST_CHECK(model.text(child1, 1) == L"1");
    BOOST_CHECK(model.text(child1, 2) == L"aaa");

    BOOST_REQUIRE(model.childs_size(child1) == 1);

    auto child2 = model.child(child1, 0);
    BOOST_REQUIRE(child2);
    BOOST_CHECK(model.text(child2, 0) == L"");
    BOOST_CHECK(model.text(child2, 1) == L"2");
    BOOST_CHECK(model.text(child2, 2) == L"bbb");
    BOOST_CHECK(model.childs_size(child2) == 0);

    auto child3 = model.child({}, 1);
    BOOST_REQUIRE(child3);
    BOOST_CHECK(model.text(child3, 0) == L"");
    BOOST_CHECK(model.text(child3, 1) == L"...");
    BOOST_CHECK(model.text(child3, 2) == L"...");
    BOOST_CHECK(model.childs_size(child3) == 0);
}


/// Tests removing first grouped item from call stack
BOOST_AUTO_TEST_CASE(test_remove_first_grouped_item) {
    {
        model.group_functions_lists_changed("(^aaa|^bbb|^ccc)"); // bbb and ccc are grouped items
        // structure will be  aaa->(bbb, ccc), ddd

        thread_info tinfo{100};
        tinfo.add_stack_frame({{400, 400, "aaa"}, 4000, 0});
        tinfo.add_stack_frame({{500, 500, "bbb"}, 5000, 0});
        tinfo.add_stack_frame({{600, 600, "ccc"}, 6000, 0});
        tinfo.add_stack_frame({{700, 700, "ddd"}, 7000, 0});

        thrd.update(tinfo);
    }


    {
        thread_info tinfo{100};
        tinfo.add_stack_frame({{500, 500, "bbb"}, 5000, 0});
        tinfo.add_stack_frame({{600, 600, "ccc"}, 6000, 0});
        tinfo.add_stack_frame({{700, 700, "ddd"}, 7000, 0});
        thrd.update(tinfo);
    }


    BOOST_REQUIRE(model.childs_size({}) == 2);

    auto child1 = model.child({}, 0);
    BOOST_REQUIRE(child1);
    BOOST_CHECK(model.text(child1, 0) == L"");
    BOOST_CHECK(model.text(child1, 1) == L"1");
    BOOST_CHECK(model.text(child1, 2) == L"bbb");

    BOOST_REQUIRE(model.childs_size(child1) == 1);

    auto child2 = model.child(child1, 0);
    BOOST_REQUIRE(child2);
    BOOST_CHECK(model.text(child2, 0) == L"");
    BOOST_CHECK(model.text(child2, 1) == L"2");
    BOOST_CHECK(model.text(child2, 2) == L"ccc");
    BOOST_CHECK(model.childs_size(child2) == 0);

    auto child4 = model.child({}, 1);
    BOOST_REQUIRE(child4);
    BOOST_CHECK(model.text(child4, 0) == L"");
    BOOST_CHECK(model.text(child4, 1) == L"3");
    BOOST_CHECK(model.text(child4, 2) == L"ddd");
    BOOST_CHECK(model.childs_size(child4) == 0);
}


/// Tests adding first grouped item into call stack
BOOST_AUTO_TEST_CASE(test_add_first_grouped_item) {
    {
        model.group_functions_lists_changed("(^aaa|^bbb|^ccc)"); // bbb and ccc are grouped items
        // structure will be  aaa->(bbb, ccc), ddd

        thread_info tinfo{100};
        tinfo.add_stack_frame({{400, 400, "aaa"}, 4000, 0});
        tinfo.add_stack_frame({{500, 500, "bbb"}, 5000, 0});
        tinfo.add_stack_frame({{600, 600, "ccc"}, 6000, 0});
        tinfo.add_stack_frame({{700, 700, "ddd"}, 7000, 0});

        thrd.update(tinfo);
    }


    {
        thread_info tinfo{100};
        tinfo.add_stack_frame({{300, 300, "xxx"}, 4000, 0});
        tinfo.add_stack_frame({{400, 400, "aaa"}, 4000, 0});
        tinfo.add_stack_frame({{500, 500, "bbb"}, 5000, 0});
        tinfo.add_stack_frame({{600, 600, "ccc"}, 6000, 0});
        tinfo.add_stack_frame({{700, 700, "ddd"}, 7000, 0});

        thrd.update(tinfo);
    }


    BOOST_REQUIRE_EQUAL(model.childs_size({}), 2);

    auto child1 = model.child({}, 0);
    BOOST_REQUIRE(child1);
    BOOST_CHECK(model.text(child1, 0) == L"");
    BOOST_CHECK(model.text(child1, 1) == L"1");
    BOOST_CHECK(model.text(child1, 2) == L"xxx");

    BOOST_REQUIRE(model.childs_size(child1) == 3);

    auto child2 = model.child(child1, 0);
    BOOST_REQUIRE(child2);
    BOOST_CHECK(model.text(child2, 0) == L"");
    BOOST_CHECK(model.text(child2, 1) == L"2");
    BOOST_CHECK(model.text(child2, 2) == L"aaa");
    BOOST_CHECK(model.childs_size(child2) == 0);
    
    auto child3 = model.child(child1, 1);
    BOOST_REQUIRE(child3);
    BOOST_CHECK(model.text(child3, 0) == L"");
    BOOST_CHECK(model.text(child3, 1) == L"3");
    BOOST_CHECK(model.text(child3, 2) == L"bbb");
    BOOST_CHECK(model.childs_size(child3) == 0);

    auto child4 = model.child(child1, 2);
    BOOST_REQUIRE(child4);
    BOOST_CHECK(model.text(child4, 0) == L"");
    BOOST_CHECK(model.text(child4, 1) == L"4");
    BOOST_CHECK(model.text(child4, 2) == L"ccc");
    BOOST_CHECK(model.childs_size(child4) == 0);


    auto child5 = model.child({}, 1);
    BOOST_REQUIRE(child5);
    BOOST_CHECK(model.text(child5, 0) == L"");
    BOOST_CHECK(model.text(child5, 1) == L"5");
    BOOST_CHECK(model.text(child5, 2) == L"ddd");
    BOOST_CHECK(model.childs_size(child5) == 0);
}


BOOST_AUTO_TEST_SUITE_END()


}
