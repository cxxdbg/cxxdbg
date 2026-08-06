// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread_test.cpp
/// Contains unit tests for the thread class.

#include "cxxdbg/dbg/source_model.hpp"
#include "cxxdbg/dbg/stack_frame.hpp"
#include "cxxdbg/dbg/stack_frame_info.hpp"
#include "cxxdbg/dbg/thread.hpp"
#include "cxxdbg/dbg/thread_info.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::test {


struct thread_test_fixture {
    source_model sources;
};


BOOST_FIXTURE_TEST_SUITE(thread_test, thread_test_fixture)


/// Tests thread constructor
BOOST_AUTO_TEST_CASE(ctor) {
    thread thrd{sources, 200};
    BOOST_CHECK(thrd.id() == 200);
    BOOST_CHECK(thrd.call_stack_size() == 0);
    BOOST_CHECK(std::ranges::empty(thrd.call_stack()));
    BOOST_CHECK(thrd.current_frame_index() == thread::invalid_current_frame);
    BOOST_CHECK(thrd.has_complete_call_stack());
}


/// Tests updating empty thread with thread info with complete call stack
BOOST_AUTO_TEST_CASE(update_empty_complete) {
    thread thrd{sources, 200};

    thread_info tinfo{200};
    tinfo.add_stack_frame({{100, 100, "func", {}}, 4000, 0});
    tinfo.set_has_complete_call_stack(true);

    thrd.before_stack_frames_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    thrd.after_stack_frames_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    unsigned int sig_num = 0;

    thrd.before_stack_frames_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    thrd.after_stack_frames_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    thrd.before_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 0);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 0);
        BOOST_CHECK(thrd.call_stack().size() == 0);
        BOOST_CHECK(thrd.current_frame_index() == thread::invalid_current_frame);
    });

    thrd.after_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 1);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 0);

        BOOST_REQUIRE(thrd.call_stack_size() == 1);
        BOOST_CHECK(thrd.current_frame_index() == thread::invalid_current_frame);

        auto frm = thrd.frame_at(0);
        BOOST_REQUIRE(frm);

        BOOST_CHECK(frm->pos().file_addr() == 100);
        BOOST_CHECK(frm->pos().load_addr() == 100);
        BOOST_CHECK(frm->pos().func_name() == "func");
        BOOST_CHECK(!frm->pos().src_pos().is_valid());
    });

    thrd.stack_frame_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    thrd.update(tinfo);

    BOOST_CHECK(sig_num == 2);

    BOOST_CHECK(thrd.has_complete_call_stack());
    BOOST_REQUIRE(thrd.call_stack_size() == 1);
    BOOST_CHECK(thrd.current_frame_index() == thread::invalid_current_frame);

    auto frm = thrd.frame_at(0);
    BOOST_REQUIRE(frm);

    BOOST_CHECK(frm->pos().file_addr() == 100);
    BOOST_CHECK(frm->pos().load_addr() == 100);
    BOOST_CHECK(frm->pos().func_name() == "func");
    BOOST_CHECK(!frm->pos().src_pos().is_valid());
}


/// Tests updating empty thread with thread info with incomplete call stack
BOOST_AUTO_TEST_CASE(update_empty_partial) {
    thread thrd{sources, 200};

    thread_info tinfo{200};
    tinfo.add_stack_frame({{100, 100, "func", {}}, 4000, 0});
    tinfo.set_has_complete_call_stack(false);

    unsigned int sig_num = 0;

    thrd.before_stack_frames_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    thrd.after_stack_frames_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    thrd.before_stack_frames_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    thrd.after_stack_frames_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    thrd.before_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 2);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 0);
        BOOST_CHECK(thrd.call_stack().size() == 0);
        BOOST_CHECK(thrd.current_frame_index() == thread::invalid_current_frame);
    });

    thrd.after_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 3);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 0);

        BOOST_REQUIRE(thrd.call_stack().size() == 1);
        BOOST_CHECK(thrd.current_frame_index() == thread::invalid_current_frame);
    });

    thrd.stack_frame_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    thrd.before_complete_call_stack_changed.connect([&thrd, &sig_num] {
        BOOST_CHECK(sig_num == 0);
        ++sig_num;

        BOOST_CHECK(thrd.has_complete_call_stack());
    });

    thrd.after_complete_call_stack_changed.connect([&thrd, &sig_num] {
        BOOST_CHECK(sig_num == 1);
        ++sig_num;

        BOOST_CHECK(!thrd.has_complete_call_stack());
    });

    thrd.update(tinfo);

    BOOST_CHECK(sig_num == 4);

    BOOST_CHECK(!thrd.has_complete_call_stack());
    BOOST_REQUIRE(thrd.call_stack_size() == 1);
    BOOST_CHECK(thrd.current_frame_index() == thread::invalid_current_frame);

    auto frm = thrd.frame_at(0);
    BOOST_REQUIRE(frm);

    BOOST_CHECK(frm->pos().file_addr() == 100);
    BOOST_CHECK(frm->pos().load_addr() == 100);
    BOOST_CHECK(frm->pos().func_name() == "func");
    BOOST_CHECK(!frm->pos().src_pos().is_valid());
}


/// Tests updating call stack with same frames
/// |+++|
/// |+++|
BOOST_AUTO_TEST_CASE(update_same) {
    thread thrd{sources, 200};

    thread_info tinfo{200};
    tinfo.add_stack_frame({{100, 100, "func", {}}, 4000, 0});
    tinfo.add_stack_frame({{200, 200, "func2", {}}, 5000, 0});
    tinfo.add_stack_frame({{100, 100, "main", {}}, 6000, 0});
    thrd.update(tinfo);
    thrd.set_current_frame_index(0);

    unsigned int sig_num = 0;

    thrd.before_stack_frames_removed.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(false);
    });

    thrd.after_stack_frames_removed.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(false);
    });

    thrd.before_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(false);
    });

    thrd.after_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(false);
    });

    thrd.stack_frame_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    thrd.update(tinfo);

    BOOST_REQUIRE_EQUAL(thrd.call_stack().size(), 3);
    BOOST_CHECK_EQUAL(thrd.current_frame_index(), 0);

    auto frm1 = thrd.call_stack()[0];
    BOOST_CHECK((frm1->pos() == code_position{100, 100, "func"}));

    auto frm2 = thrd.call_stack()[1];
    BOOST_CHECK((frm2->pos() == code_position{200, 200, "func2"}));

    auto frm3 = thrd.call_stack()[2];
    BOOST_CHECK((frm3->pos() == code_position{100, 100, "main"}));
}



/// Tests updating call stack with no overlap:
/// |---------|
///               |--------|
BOOST_AUTO_TEST_CASE(update_no_overlap) {
    thread thrd{sources, 200};

    thread_info tinfo{200};
    tinfo.add_stack_frame({{100, 100, "func", {}}, 4000, 0});
    tinfo.add_stack_frame({{200, 200, "func2", {}}, 5000, 0});
    tinfo.add_stack_frame({{100, 100, "main", {}}, 6000, 0});
    thrd.update(tinfo);
    thrd.set_current_frame_index(0);

    unsigned int sig_num = 0;

    thrd.before_stack_frames_removed.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 0);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 2);

        BOOST_REQUIRE(thrd.call_stack().size() == 3);
        BOOST_CHECK(thrd.current_frame_index() == 0);

        auto frm1 = thrd.call_stack()[0];
        BOOST_CHECK((frm1->pos() == code_position{100, 100, "func"}));

        auto frm2 = thrd.call_stack()[1];
        BOOST_CHECK((frm2->pos() == code_position{200, 200, "func2"}));

        auto frm3 = thrd.call_stack()[2];
        BOOST_CHECK((frm3->pos() == code_position{100, 100, "main"}));
    });

    thrd.after_stack_frames_removed.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 1);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 2);

        BOOST_CHECK(thrd.call_stack().size() == 0);
        BOOST_CHECK(thrd.current_frame_index() == thread::invalid_current_frame);
    });

    thrd.before_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 2);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 2);

        BOOST_CHECK(thrd.call_stack().size() == 0);
        BOOST_CHECK(thrd.current_frame_index() == thread::invalid_current_frame);
    });

    thrd.after_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 3);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 2);

        BOOST_REQUIRE(thrd.call_stack().size() == 3);
        BOOST_CHECK(thrd.current_frame_index() == thread::invalid_current_frame);

        auto frm1 = thrd.call_stack()[0];
        BOOST_CHECK((frm1->pos() == code_position{300, 300, "xxx"}));

        auto frm2 = thrd.call_stack()[1];
        BOOST_CHECK((frm2->pos() == code_position{400, 400, "yyy"}));

        auto frm3 = thrd.call_stack()[2];
        BOOST_CHECK((frm3->pos() == code_position{320, 320, "zzz"}));
    });

    thrd.stack_frame_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    thread_info tinfo2{200};
    tinfo2.add_stack_frame({{300, 300, "xxx"}, 7000, 0});
    tinfo2.add_stack_frame({{400, 400, "yyy"}, 8000, 0});
    tinfo2.add_stack_frame({{320, 320, "zzz"}, 9000, 0});
    thrd.update(tinfo2);

    BOOST_CHECK(sig_num == 4);

    BOOST_REQUIRE(thrd.call_stack().size() == 3);
    BOOST_CHECK(thrd.current_frame_index() == thread::invalid_current_frame);

    auto frm1 = thrd.call_stack()[0];
    BOOST_CHECK((frm1->pos() == code_position{300, 300, "xxx"}));

    auto frm2 = thrd.call_stack()[1];
    BOOST_CHECK((frm2->pos() == code_position{400, 400, "yyy"}));

    auto frm3 = thrd.call_stack()[2];
    BOOST_CHECK((frm3->pos() == code_position{320, 320, "zzz"}));
}


/// Tests updating call stack with no overlap:
///               |---------|
/// |--------|
BOOST_AUTO_TEST_CASE(update_no_overlap_2) {
    thread thrd{sources, 200};

    thread_info tinfo{200};
    tinfo.add_stack_frame({{100, 100, "func", {}}, 7000, 0});
    tinfo.add_stack_frame({{200, 200, "func2", {}}, 8000, 0});
    tinfo.add_stack_frame({{100, 100, "main", {}}, 9000, 0});
    thrd.update(tinfo);
    thrd.set_current_frame_index(0);

    unsigned int sig_num = 0;

    thrd.before_stack_frames_removed.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 0);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 2);

        BOOST_REQUIRE(thrd.call_stack().size() == 3);
        BOOST_CHECK(thrd.current_frame_index() == 0);

        auto frm1 = thrd.call_stack()[0];
        BOOST_CHECK((frm1->pos() == code_position{100, 100, "func"}));

        auto frm2 = thrd.call_stack()[1];
        BOOST_CHECK((frm2->pos() == code_position{200, 200, "func2"}));

        auto frm3 = thrd.call_stack()[2];
        BOOST_CHECK((frm3->pos() == code_position{100, 100, "main"}));
    });

    thrd.after_stack_frames_removed.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 1);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 2);

        BOOST_CHECK(thrd.call_stack().size() == 0);
        BOOST_CHECK(thrd.current_frame_index() == thread::invalid_current_frame);
    });

    thrd.before_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 2);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 2);

        BOOST_CHECK(thrd.call_stack().size() == 0);
        BOOST_CHECK(thrd.current_frame_index() == thread::invalid_current_frame);
    });

    thrd.after_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 3);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 2);

        BOOST_REQUIRE(thrd.call_stack().size() == 3);
        BOOST_CHECK(thrd.current_frame_index() == thread::invalid_current_frame);

        auto frm1 = thrd.call_stack()[0];
        BOOST_CHECK((frm1->pos() == code_position{300, 300, "xxx"}));

        auto frm2 = thrd.call_stack()[1];
        BOOST_CHECK((frm2->pos() == code_position{400, 400, "yyy"}));

        auto frm3 = thrd.call_stack()[2];
        BOOST_CHECK((frm3->pos() == code_position{320, 320, "zzz"}));
    });

    thrd.stack_frame_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    thread_info tinfo2{200};
    tinfo2.add_stack_frame({{300, 300, "xxx"}, 4000, 0});
    tinfo2.add_stack_frame({{400, 400, "yyy"}, 5000, 0});
    tinfo2.add_stack_frame({{320, 320, "zzz"}, 6000, 0});
    thrd.update(tinfo2);

    BOOST_CHECK(sig_num == 4);

    BOOST_REQUIRE(thrd.call_stack().size() == 3);
    BOOST_CHECK(thrd.current_frame_index() == thread::invalid_current_frame);

    auto frm1 = thrd.call_stack()[0];
    BOOST_CHECK((frm1->pos() == code_position{300, 300, "xxx"}));

    auto frm2 = thrd.call_stack()[1];
    BOOST_CHECK((frm2->pos() == code_position{400, 400, "yyy"}));

    auto frm3 = thrd.call_stack()[2];
    BOOST_CHECK((frm3->pos() == code_position{320, 320, "zzz"}));
}


/// Tests updating call stack with overlap at the end of new stack:
///    |-+-|
///   |--+|
BOOST_AUTO_TEST_CASE(update_overlap_begin) {
    thread thrd{sources, 200};

    thread_info tinfo{200};
    tinfo.add_stack_frame({{100, 100, "func", {}}, 4000, 0});
    tinfo.add_stack_frame({{200, 200, "func2", {}}, 5000, 0});
    tinfo.add_stack_frame({{100, 100, "main", {}}, 6000, 0});
    thrd.update(tinfo);
    thrd.set_current_frame_index(1);

    unsigned int sig_num = 0;

    thrd.before_stack_frames_removed.connect([&sig_num, &thrd](size_t f, size_t l) {
        if (sig_num == 0) {
            // begin of call stack

            BOOST_CHECK(f == 0);
            BOOST_CHECK(l == 0);

            BOOST_REQUIRE(thrd.call_stack().size() == 3);
            BOOST_CHECK(thrd.current_frame_index() == 1);

            auto frm1 = thrd.call_stack()[0];
            BOOST_REQUIRE(frm1);
            BOOST_CHECK((frm1->pos() == code_position{100, 100, "func"}));

            auto frm2 = thrd.call_stack()[1];
            BOOST_REQUIRE(frm2);
            BOOST_CHECK((frm2->pos() == code_position{200, 200, "func2"}));

            auto frm3 = thrd.call_stack()[2];
            BOOST_REQUIRE(frm3);
            BOOST_CHECK((frm3->pos() == code_position{100, 100, "main"}));

        } else if (sig_num == 2) {
            // end of call stack

            BOOST_CHECK(f == 1);
            BOOST_CHECK(l == 1);

            BOOST_REQUIRE(thrd.call_stack().size() == 2);
            BOOST_CHECK(thrd.current_frame_index() == 0);

            auto frm1 = thrd.call_stack()[0];
            BOOST_REQUIRE(frm1);
            BOOST_CHECK((frm1->pos() == code_position{200, 200, "func2"}));

            auto frm2 = thrd.call_stack()[1];
            BOOST_REQUIRE(frm2);
            BOOST_CHECK((frm2->pos() == code_position{100, 100, "main"}));

        } else {
            BOOST_CHECK(false);
        }

        ++sig_num;
    });

    thrd.after_stack_frames_removed.connect([&sig_num, &thrd](size_t f, size_t l) {
        if (sig_num == 1) {
            // begin of call stack

            BOOST_CHECK(f == 0);
            BOOST_CHECK(l == 0);

            BOOST_REQUIRE(thrd.call_stack().size() == 2);
            BOOST_CHECK(thrd.current_frame_index() == 0);

            auto frm1 = thrd.call_stack()[0];
            BOOST_REQUIRE(frm1);
            BOOST_CHECK((frm1->pos() == code_position{200, 200, "func2"}));

            auto frm2 = thrd.call_stack()[1];
            BOOST_REQUIRE(frm2);
            BOOST_CHECK((frm2->pos() == code_position{100, 100, "main"}));

        } else if (sig_num == 3) {
            // end of call stack

            BOOST_CHECK(f == 1);
            BOOST_CHECK(l == 1);

            BOOST_REQUIRE(thrd.call_stack().size() == 1);
            BOOST_CHECK(thrd.current_frame_index() == 0);

            auto frm1 = thrd.call_stack()[0];
            BOOST_REQUIRE(frm1);
            BOOST_CHECK((frm1->pos() == code_position{200, 200, "func2"}));

        } else {
            BOOST_CHECK(false);
        }

        ++sig_num;
    });

    thrd.before_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 4);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 1);

        BOOST_REQUIRE(thrd.call_stack().size() == 1);
        BOOST_CHECK(thrd.current_frame_index() == 0);

        auto frm1 = thrd.call_stack()[0];
        BOOST_REQUIRE(frm1);
        BOOST_CHECK((frm1->pos() == code_position{200, 200, "func2"}));
    });

    thrd.after_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 5);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 1);

        BOOST_REQUIRE(thrd.call_stack().size() == 3);
        BOOST_CHECK(thrd.current_frame_index() == 2);

        auto frm1 = thrd.call_stack()[0];
        BOOST_CHECK((frm1->pos() == code_position{120, 120, "xxx"}));

        auto frm2 = thrd.call_stack()[1];
        BOOST_CHECK((frm2->pos() == code_position{100, 100, "yyy"}));

        auto frm3 = thrd.call_stack()[2];
        BOOST_CHECK((frm3->pos() == code_position{200, 200, "func2"}));
    });

    thrd.stack_frame_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    thread_info tinfo2{200};
    tinfo2.add_stack_frame({{120, 120, "xxx", {}}, 4300, 0});
    tinfo2.add_stack_frame({{100, 100, "yyy", {}}, 4400, 0});
    tinfo2.add_stack_frame({{200, 200, "func2", {}}, 5000, 0});
    thrd.update(tinfo2);

    BOOST_CHECK(sig_num == 6);

    BOOST_REQUIRE(thrd.call_stack().size() == 3);
    BOOST_CHECK(thrd.current_frame_index() == 2);

    auto frm1 = thrd.call_stack()[0];
    BOOST_CHECK((frm1->pos() == code_position{120, 120, "xxx"}));

    auto frm2 = thrd.call_stack()[1];
    BOOST_CHECK((frm2->pos() == code_position{100, 100, "yyy"}));

    auto frm3 = thrd.call_stack()[2];
    BOOST_CHECK((frm3->pos() == code_position{200, 200, "func2"}));
}


/// Tests updating call stack with overlap at end of old stack:
/// |--+|
///  |-+-|
BOOST_AUTO_TEST_CASE(update_overlap_end) {
    thread thrd{sources, 200};

    thread_info tinfo{200};
    tinfo.add_stack_frame({{100, 100, "func", {}}, 4000, 0});
    tinfo.add_stack_frame({{200, 200, "func2", {}}, 5000, 0});
    tinfo.add_stack_frame({{100, 100, "main", {}}, 6000, 0});
    thrd.update(tinfo);
    thrd.set_current_frame_index(2);

    unsigned int sig_num = 0;

    thrd.before_stack_frames_removed.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 0);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 1);

        BOOST_REQUIRE(thrd.call_stack().size() == 3);
        BOOST_CHECK(thrd.current_frame_index() == 2);

        auto frm1 = thrd.call_stack()[0];
        BOOST_REQUIRE(frm1);
        BOOST_CHECK((frm1->pos() == code_position{100, 100, "func"}));

        auto frm2 = thrd.call_stack()[1];
        BOOST_REQUIRE(frm2);
        BOOST_CHECK((frm2->pos() == code_position{200, 200, "func2"}));

        auto frm3 = thrd.call_stack()[2];
        BOOST_REQUIRE(frm3);
        BOOST_CHECK((frm3->pos() == code_position{100, 100, "main"}));
    });

    thrd.after_stack_frames_removed.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 1);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 1);

        BOOST_REQUIRE(thrd.call_stack().size() == 1);
        BOOST_CHECK(thrd.current_frame_index() == 0);

        auto frm1 = thrd.call_stack()[0];
        BOOST_REQUIRE(frm1);
        BOOST_CHECK((frm1->pos() == code_position{100, 100, "main"}));
    });

    thrd.before_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        if (sig_num == 2) {
            // begin of stack

            BOOST_CHECK(f == 0);
            BOOST_CHECK(l == 0);

            BOOST_REQUIRE(thrd.call_stack().size() == 1);
            BOOST_CHECK(thrd.current_frame_index() == 0);

            auto frm1 = thrd.call_stack()[0];
            BOOST_REQUIRE(frm1);
            BOOST_CHECK((frm1->pos() == code_position{100, 100, "main"}));

        } else if (sig_num == 4) {
            // end of stack

            BOOST_CHECK(f == 2);
            BOOST_CHECK(l == 2);

            BOOST_REQUIRE(thrd.call_stack().size() == 2);
            BOOST_CHECK(thrd.current_frame_index() == 1);

            auto frm1 = thrd.call_stack()[0];
            BOOST_REQUIRE(frm1);
            BOOST_CHECK((frm1->pos() == code_position{110, 110, "foo"}));

            auto frm2 = thrd.call_stack()[1];
            BOOST_REQUIRE(frm2);
            BOOST_CHECK((frm2->pos() == code_position{100, 100, "main"}));

        } else {
            BOOST_CHECK(false);
        }

        ++sig_num;
    });

    thrd.after_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        if (sig_num == 3) {
            // begin of stack

            BOOST_CHECK(f == 0);
            BOOST_CHECK(l == 0);

            BOOST_REQUIRE(thrd.call_stack().size() == 2);
            BOOST_CHECK(thrd.current_frame_index() == 1);

            auto frm1 = thrd.call_stack()[0];
            BOOST_REQUIRE(frm1);
            BOOST_CHECK((frm1->pos() == code_position{110, 110, "foo"}));

            auto frm2 = thrd.call_stack()[1];
            BOOST_REQUIRE(frm2);
            BOOST_CHECK((frm2->pos() == code_position{100, 100, "main"}));

        } else if (sig_num == 5) {
            // end of stack

            BOOST_CHECK(f == 2);
            BOOST_CHECK(l == 2);

            BOOST_REQUIRE(thrd.call_stack().size() == 3);
            BOOST_CHECK(thrd.current_frame_index() == 1);

            auto frm1 = thrd.call_stack()[0];
            BOOST_REQUIRE(frm1);
            BOOST_CHECK((frm1->pos() == code_position{110, 110, "foo"}));

            auto frm2 = thrd.call_stack()[1];
            BOOST_REQUIRE(frm2);
            BOOST_CHECK((frm2->pos() == code_position{100, 100, "main"}));

            auto frm3 = thrd.call_stack()[2];
            BOOST_REQUIRE(frm3);
            BOOST_CHECK((frm3->pos() == code_position{90, 90, "start"}));
        } else {
            BOOST_CHECK(false);
        }

        ++sig_num;
    });

    thrd.stack_frame_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    thread_info tinfo2{200};
    tinfo2.add_stack_frame({{110, 110, "foo"}, 4000, 0});
    tinfo2.add_stack_frame({{100, 100, "main"}, 6000, 0});
    tinfo2.add_stack_frame({{90, 90, "start"}, 6200, 0});
    thrd.update(tinfo2);

    BOOST_CHECK(sig_num == 6);

    BOOST_REQUIRE(thrd.call_stack().size() == 3);
    BOOST_CHECK(thrd.current_frame_index() == 1);

    auto frm1 = thrd.call_stack()[0];
    BOOST_REQUIRE(frm1);
    BOOST_CHECK((frm1->pos() == code_position{110, 110, "foo"}));

    auto frm2 = thrd.call_stack()[1];
    BOOST_REQUIRE(frm2);
    BOOST_CHECK((frm2->pos() == code_position{100, 100, "main"}));

    auto frm3 = thrd.call_stack()[2];
    BOOST_REQUIRE(frm3);
    BOOST_CHECK((frm3->pos() == code_position{90, 90, "start"}));
}


/// Tests updating call stack with overlap at middle:
///  |--+-|
///   |-+|
BOOST_AUTO_TEST_CASE(update_overlap_middle) {
    thread thrd{sources, 200};

    thread_info tinfo{200};
    tinfo.add_stack_frame({{100, 100, "foo"}, 3000, 0});
    tinfo.add_stack_frame({{90, 90, "bar"}, 4000, 0});
    tinfo.add_stack_frame({{200, 200, "main"}, 5000, 0});
    tinfo.add_stack_frame({{10, 10, "start"}, 6000, 0});
    thrd.update(tinfo);
    thrd.set_current_frame_index(2);

    unsigned int sig_num = 0;

    thrd.before_stack_frames_removed.connect([&sig_num, &thrd](size_t f, size_t l) {
        if (sig_num == 0) {
            // begin of stack

            BOOST_CHECK(f == 0);
            BOOST_CHECK(l == 1);

            BOOST_REQUIRE(thrd.call_stack().size() == 4);
            BOOST_CHECK(thrd.current_frame_index() == 2);

            auto frm1 = thrd.call_stack()[0];
            BOOST_REQUIRE(frm1);
            BOOST_CHECK((frm1->pos() == code_position{100, 100, "foo"}));

            auto frm2 = thrd.call_stack()[1];
            BOOST_REQUIRE(frm2);
            BOOST_CHECK((frm2->pos() == code_position{90, 90, "bar"}));

            auto frm3 = thrd.call_stack()[2];
            BOOST_REQUIRE(frm3);
            BOOST_CHECK((frm3->pos() == code_position{200, 200, "main"}));

            auto frm4 = thrd.call_stack()[3];
            BOOST_REQUIRE(frm4);
            BOOST_CHECK((frm4->pos() == code_position{10, 10, "start"}));

        } else if (sig_num == 2) {
            // end of stack

            BOOST_CHECK(f == 1);
            BOOST_CHECK(l == 1);

            BOOST_REQUIRE(thrd.call_stack().size() == 2);
            BOOST_CHECK(thrd.current_frame_index() == 0);

            auto frm1 = thrd.call_stack()[0];
            BOOST_REQUIRE(frm1);
            BOOST_CHECK((frm1->pos() == code_position{200, 200, "main"}));

            auto frm2 = thrd.call_stack()[1];
            BOOST_REQUIRE(frm2);
            BOOST_CHECK((frm2->pos() == code_position{10, 10, "start"}));

        } else {
            BOOST_CHECK(false);
        }

        ++sig_num;
    });

    thrd.after_stack_frames_removed.connect([&sig_num, &thrd](size_t f, size_t l) {
        if (sig_num == 1) {
            // begin of stack

            BOOST_CHECK(f == 0);
            BOOST_CHECK(l == 1);

            BOOST_REQUIRE(thrd.call_stack().size() == 2);
            BOOST_CHECK(thrd.current_frame_index() == 0);

            auto frm1 = thrd.call_stack()[0];
            BOOST_REQUIRE(frm1);
            BOOST_CHECK((frm1->pos() == code_position{200, 200, "main"}));

            auto frm2 = thrd.call_stack()[1];
            BOOST_REQUIRE(frm2);
            BOOST_CHECK((frm2->pos() == code_position{10, 10, "start"}));

        } else if (sig_num == 3) {
            // end of stack

            BOOST_CHECK(f == 1);
            BOOST_CHECK(l == 1);

            BOOST_REQUIRE(thrd.call_stack().size() == 1);
            BOOST_CHECK(thrd.current_frame_index() == 0);

            auto frm1 = thrd.call_stack()[0];
            BOOST_REQUIRE(frm1);
            BOOST_CHECK((frm1->pos() == code_position{200, 200, "main"}));
        } else {
            BOOST_CHECK(false);
        }

        ++sig_num;
    });

    thrd.before_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 4);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 0);

        BOOST_REQUIRE(thrd.call_stack().size() == 1);
        BOOST_CHECK(thrd.current_frame_index() == 0);

        auto frm1 = thrd.call_stack()[0];
        BOOST_REQUIRE(frm1);
        BOOST_CHECK((frm1->pos() == code_position{200, 200, "main"}));
    });

    thrd.after_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 5);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 0);

        BOOST_REQUIRE(thrd.call_stack().size() == 2);
        BOOST_CHECK(thrd.current_frame_index() == 1);

        auto frm1 = thrd.call_stack()[0];
        BOOST_REQUIRE(frm1);
        BOOST_CHECK((frm1->pos() == code_position{30, 30, "zzz"}));

        auto frm2 = thrd.call_stack()[1];
        BOOST_REQUIRE(frm2);
        BOOST_CHECK((frm2->pos() == code_position{200, 200, "main"}));
    });

    thrd.stack_frame_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    thread_info tinfo2{200};
    tinfo2.add_stack_frame({{30, 30, "zzz"}, 4000, 0});
    tinfo2.add_stack_frame({{200, 200, "main"}, 5000, 0});
    thrd.update(tinfo2);

    BOOST_CHECK(sig_num == 6);

    BOOST_REQUIRE(thrd.call_stack().size() == 2);
    BOOST_CHECK(thrd.current_frame_index() == 1);

    auto frm1 = thrd.call_stack()[0];
    BOOST_REQUIRE(frm1);
    BOOST_CHECK((frm1->pos() == code_position{30, 30, "zzz"}));

    auto frm2 = thrd.call_stack()[1];
    BOOST_REQUIRE(frm2);
    BOOST_CHECK((frm2->pos() == code_position{200, 200, "main"}));
}


/// Tests updating call stack with overlap in middle:
///   |-+|
///  |--+-|
BOOST_AUTO_TEST_CASE(update_overlap_middle_2) {
    thread thrd{sources, 200};

    thread_info tinfo{200};
    tinfo.add_stack_frame({{100, 100, "foo"}, 3000, 0});
    tinfo.add_stack_frame({{110, 110, "main"}, 4000, 0});
    thrd.update(tinfo);
    thrd.set_current_frame_index(1);

    unsigned int sig_num = 0;

    thrd.before_stack_frames_removed.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 0);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 0);

        BOOST_REQUIRE(thrd.call_stack().size() == 2);
        BOOST_CHECK(thrd.current_frame_index() == 1);

        auto frm1 = thrd.call_stack()[0];
        BOOST_REQUIRE(frm1);
        BOOST_CHECK((frm1->pos() == code_position{100, 100, "foo"}));

        auto frm2 = thrd.call_stack()[1];
        BOOST_REQUIRE(frm2);
        BOOST_CHECK((frm2->pos() == code_position{110, 110, "main"}));
    });

    thrd.after_stack_frames_removed.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 1);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 0);

        BOOST_REQUIRE(thrd.call_stack().size() == 1);
        BOOST_CHECK(thrd.current_frame_index() == 0);

        auto frm1 = thrd.call_stack()[0];
        BOOST_REQUIRE(frm1);
        BOOST_CHECK((frm1->pos() == code_position{110, 110, "main"}));
    });

    thrd.before_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        if (sig_num == 2) {
            // begin of stack

            BOOST_CHECK(f == 0);
            BOOST_CHECK(l == 1);

            BOOST_REQUIRE(thrd.call_stack().size() == 1);
            BOOST_CHECK(thrd.current_frame_index() == 0);

            auto frm1 = thrd.call_stack()[0];
            BOOST_REQUIRE(frm1);
            BOOST_CHECK((frm1->pos() == code_position{110, 110, "main"}));

        } else if (sig_num == 4) {
            // end of stack

            BOOST_CHECK(f == 3);
            BOOST_CHECK(l == 3);

            BOOST_REQUIRE(thrd.call_stack().size() == 3);
            BOOST_CHECK(thrd.current_frame_index() == 2);

            auto frm1 = thrd.call_stack()[0];
            BOOST_REQUIRE(frm1);
            BOOST_CHECK((frm1->pos() == code_position{120, 120, "xxx"}));

            auto frm2 = thrd.call_stack()[1];
            BOOST_REQUIRE(frm2);
            BOOST_CHECK((frm2->pos() == code_position{121, 121, "yyy"}));

            auto frm3 = thrd.call_stack()[2];
            BOOST_REQUIRE(frm3);
            BOOST_CHECK((frm3->pos() == code_position{110, 110, "main"}));
        } else {
            BOOST_CHECK(false);
        }

        ++sig_num;
    });

    thrd.after_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        if (sig_num == 3) {
            // begin of stack

            BOOST_CHECK(f == 0);
            BOOST_CHECK(l == 1);

            BOOST_REQUIRE(thrd.call_stack().size() == 3);
            BOOST_CHECK(thrd.current_frame_index() == 2);

            auto frm1 = thrd.call_stack()[0];
            BOOST_REQUIRE(frm1);
            BOOST_CHECK((frm1->pos() == code_position{120, 120, "xxx"}));

            auto frm2 = thrd.call_stack()[1];
            BOOST_REQUIRE(frm2);
            BOOST_CHECK((frm2->pos() == code_position{121, 121, "yyy"}));

            auto frm3 = thrd.call_stack()[2];
            BOOST_REQUIRE(frm3);
            BOOST_CHECK((frm3->pos() == code_position{110, 110, "main"}));

        } else if (sig_num == 5) {
            // end of stack

            BOOST_CHECK(f == 3);
            BOOST_CHECK(l == 3);

            BOOST_REQUIRE(thrd.call_stack().size() == 4);
            BOOST_CHECK(thrd.current_frame_index() == 2);

            auto frm1 = thrd.call_stack()[0];
            BOOST_REQUIRE(frm1);
            BOOST_CHECK((frm1->pos() == code_position{120, 120, "xxx"}));

            auto frm2 = thrd.call_stack()[1];
            BOOST_REQUIRE(frm2);
            BOOST_CHECK((frm2->pos() == code_position{121, 121, "yyy"}));

            auto frm3 = thrd.call_stack()[2];
            BOOST_REQUIRE(frm3);
            BOOST_CHECK((frm3->pos() == code_position{110, 110, "main"}));

            auto frm4 = thrd.call_stack()[3];
            BOOST_REQUIRE(frm4);
            BOOST_CHECK((frm4->pos() == code_position{10, 10, "start"}));

        } else {
            BOOST_CHECK(false);
        }

        ++sig_num;
    });

    thrd.stack_frame_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    thread_info tinfo2{200};
    tinfo2.add_stack_frame({{120, 120, "xxx"}, 3000, 0});
    tinfo2.add_stack_frame({{121, 121, "yyy"}, 3800, 0});
    tinfo2.add_stack_frame({{110, 110, "main"}, 4000, 0});
    tinfo2.add_stack_frame({{10, 10, "start"}, 4200, 0});
    thrd.update(tinfo2);

    BOOST_CHECK(sig_num == 6);

    BOOST_REQUIRE(thrd.call_stack().size() == 4);
    BOOST_CHECK(thrd.current_frame_index() == 2);

    auto frm1 = thrd.call_stack()[0];
    BOOST_REQUIRE(frm1);
    BOOST_CHECK((frm1->pos() == code_position{120, 120, "xxx"}));

    auto frm2 = thrd.call_stack()[1];
    BOOST_REQUIRE(frm2);
    BOOST_CHECK((frm2->pos() == code_position{121, 121, "yyy"}));

    auto frm3 = thrd.call_stack()[2];
    BOOST_REQUIRE(frm3);
    BOOST_CHECK((frm3->pos() == code_position{110, 110, "main"}));

    auto frm4 = thrd.call_stack()[3];
    BOOST_REQUIRE(frm4);
    BOOST_CHECK((frm4->pos() == code_position{10, 10, "start"}));
}


/// Tests updating call stack with same end
///  |-+|
/// |--+|
BOOST_AUTO_TEST_CASE(update_same_end) {
    thread thrd{sources, 200};

    thread_info tinfo{200};
    tinfo.add_stack_frame({{100, 100, "foo"}, 3000, 0});
    tinfo.add_stack_frame({{110, 110, "main"}, 4000, 0});
    thrd.update(tinfo);
    thrd.set_current_frame_index(1);

    unsigned int sig_num = 0;

    thrd.before_stack_frames_removed.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 0);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 0);

        BOOST_REQUIRE(thrd.call_stack().size() == 2);
        BOOST_CHECK(thrd.current_frame_index() == 1);

        auto frm1 = thrd.call_stack()[0];
        BOOST_REQUIRE(frm1);
        BOOST_CHECK((frm1->pos() == code_position{100, 100, "foo"}));

        auto frm2 = thrd.call_stack()[1];
        BOOST_REQUIRE(frm2);
        BOOST_CHECK((frm2->pos() == code_position{110, 110, "main"}));
    });

    thrd.after_stack_frames_removed.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 1);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 0);

        BOOST_REQUIRE(thrd.call_stack().size() == 1);
        BOOST_CHECK(thrd.current_frame_index() == 0);

        auto frm1 = thrd.call_stack()[0];
        BOOST_REQUIRE(frm1);
        BOOST_CHECK((frm1->pos() == code_position{110, 110, "main"}));
    });

    thrd.before_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 2);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 1);

        BOOST_REQUIRE(thrd.call_stack().size() == 1);
        BOOST_CHECK(thrd.current_frame_index() == 0);

        auto frm1 = thrd.call_stack()[0];
        BOOST_REQUIRE(frm1);
        BOOST_CHECK((frm1->pos() == code_position{110, 110, "main"}));
    });

    thrd.after_stack_frames_added.connect([&sig_num, &thrd](size_t f, size_t l) {
        BOOST_CHECK(sig_num == 3);
        ++sig_num;

        BOOST_CHECK(f == 0);
        BOOST_CHECK(l == 1);

        BOOST_REQUIRE(thrd.call_stack().size() == 3);
        BOOST_CHECK(thrd.current_frame_index() == 2);

        auto frm1 = thrd.call_stack()[0];
        BOOST_REQUIRE(frm1);
        BOOST_CHECK((frm1->pos() == code_position{120, 120, "xxx"}));

        auto frm2 = thrd.call_stack()[1];
        BOOST_REQUIRE(frm2);
        BOOST_CHECK((frm2->pos() == code_position{121, 121, "yyy"}));

        auto frm3 = thrd.call_stack()[2];
        BOOST_REQUIRE(frm3);
        BOOST_CHECK((frm3->pos() == code_position{110, 110, "main"}));
    });

    thrd.stack_frame_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    thread_info tinfo2{200};
    tinfo2.add_stack_frame({{120, 120, "xxx"}, 3000, 0});
    tinfo2.add_stack_frame({{121, 121, "yyy"}, 3800, 0});
    tinfo2.add_stack_frame({{110, 110, "main"}, 4000, 0});
    thrd.update(tinfo2);

    BOOST_REQUIRE(thrd.call_stack().size() == 3);
    BOOST_CHECK(thrd.current_frame_index() == 2);

    auto frm1 = thrd.call_stack()[0];
    BOOST_REQUIRE(frm1);
    BOOST_CHECK((frm1->pos() == code_position{120, 120, "xxx"}));

    auto frm2 = thrd.call_stack()[1];
    BOOST_REQUIRE(frm2);
    BOOST_CHECK((frm2->pos() == code_position{121, 121, "yyy"}));

    auto frm3 = thrd.call_stack()[2];
    BOOST_REQUIRE(frm3);
    BOOST_CHECK((frm3->pos() == code_position{110, 110, "main"}));

}


/// Tests updating parameters in call stack
BOOST_AUTO_TEST_CASE(update_params) {
    thread thrd{sources, 200};

    {
        thread_info tinfo{200};
        tinfo.add_stack_frame({{100, 100, "foo"}, 1000, 0});
        thrd.update(tinfo);
    }

    thrd.set_current_frame_index(0);

    thrd.before_stack_frames_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    thrd.after_stack_frames_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    thrd.before_stack_frames_added.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    thrd.after_stack_frames_added.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    unsigned int sig_num = 0;
    thrd.stack_frame_changed.connect([&sig_num, &thrd](size_t i) {
        BOOST_CHECK(sig_num == 0);
        ++sig_num;

        BOOST_CHECK(i == 0);

        BOOST_REQUIRE(thrd.call_stack().size() == 1);
        BOOST_CHECK(thrd.current_frame_index() == 0);

        auto frm1 = thrd.call_stack()[0];
        BOOST_REQUIRE(frm1);
        BOOST_CHECK((frm1->pos() == code_position{100, 100, "foo"}));

        BOOST_REQUIRE(frm1->params().size() == 1);
        BOOST_CHECK(frm1->params()[0].name() == L"x");
        BOOST_CHECK(frm1->params()[0].type() == L"int");
        BOOST_CHECK(frm1->params()[0].value() == L"100");
    });


    {
        thread_info tinfo{200};
        stack_frame_info finfo{{100, 100, "foo"}, 1000, 0};
        finfo.add_param({L"x", L"int", L"100"});
        tinfo.add_stack_frame(finfo);
        thrd.update(tinfo);
    }

    BOOST_CHECK(sig_num == 1);

    BOOST_REQUIRE(thrd.call_stack().size() == 1);
    BOOST_CHECK(thrd.current_frame_index() == 0);

    auto frm1 = thrd.call_stack()[0];
    BOOST_REQUIRE(frm1);
    BOOST_CHECK((frm1->pos() == code_position{100, 100, "foo"}));

    BOOST_REQUIRE(frm1->params().size() == 1);
    BOOST_CHECK(frm1->params()[0].name() == L"x");
    BOOST_CHECK(frm1->params()[0].type() == L"int");
    BOOST_CHECK(frm1->params()[0].value() == L"100");
}


/// Tests updating current frame
BOOST_AUTO_TEST_CASE(set_current_frame) {
    thread thrd{sources, 100};
    {
        thread_info tinfo{100};
        tinfo.add_stack_frame({{100, 100, "foo"}, 1000, 0});
        thrd.update(tinfo);
    }

    unsigned int sig_num = 0;
    thrd.stack_frame_changed.connect([&sig_num, &thrd](size_t idx) {
        BOOST_CHECK(sig_num == 0);
        ++sig_num;

        BOOST_CHECK(idx == 0);
        BOOST_CHECK(thrd.current_frame_index() == 0);
    });

    thrd.set_current_frame_index(0);

    BOOST_CHECK(sig_num == 1);
    BOOST_CHECK(thrd.current_frame_index() == 0);
}


/// Tests updating current frame 2
BOOST_AUTO_TEST_CASE(set_current_frame_2) {
    thread thrd{sources, 100};
    {
        thread_info tinfo{100};
        tinfo.add_stack_frame({{100, 100, "foo"}, 1000, 0});
        tinfo.add_stack_frame({{200, 200, "bar"}, 2000, 0});
        thrd.update(tinfo);
        thrd.set_current_frame_index(0);
    }

    unsigned int sig_num = 0;
    thrd.stack_frame_changed.connect([&sig_num, &thrd](size_t idx) {

        if (sig_num == 0) {
            BOOST_CHECK(idx == 0);
            BOOST_CHECK(thrd.current_frame_index() == 1);
        } else if (sig_num == 1) {
            BOOST_CHECK(idx == 1);
            BOOST_CHECK(thrd.current_frame_index() == 1);
        } else {
            BOOST_CHECK(false);
        }

        ++sig_num;
    });

    thrd.set_current_frame_index(1);

    BOOST_CHECK(sig_num == 2);
    BOOST_CHECK(thrd.current_frame_index() == 1);
}


/// Tests updating source position in existing frame. This may
/// happen after returning to parent function because code position
/// of parent frame is same as return address
/// CXXDBG-335
BOOST_AUTO_TEST_CASE(update_src_pos) {
    thread thrd{sources, 200};

    {
        thread_info tinfo{200};
        tinfo.add_stack_frame({{100, 100, "foo", {"test.cpp", 10}}, 1000, 0});
        tinfo.add_stack_frame({{100, 100, "main", {"test.cpp", 20}}, 1100, 0});
        thrd.update(tinfo);
    }

    int before_removed_signum = 0;
    thrd.before_stack_frames_removed.connect([&before_removed_signum](size_t f, size_t l) {
        BOOST_CHECK_EQUAL(before_removed_signum, 0);
        ++before_removed_signum;
        BOOST_CHECK_EQUAL(f, 0);
        BOOST_CHECK_EQUAL(l, 0);
    });

    int after_removed_signum = 0;
    thrd.after_stack_frames_removed.connect([&after_removed_signum](size_t f, size_t l) {
        BOOST_CHECK_EQUAL(after_removed_signum, 0);
        ++after_removed_signum;
        BOOST_CHECK_EQUAL(f, 0);
        BOOST_CHECK_EQUAL(l, 0);
    });

    thrd.before_stack_frames_added.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    thrd.after_stack_frames_added.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    unsigned int frame_changed_signum = 0;
    thrd.stack_frame_changed.connect([&frame_changed_signum, &thrd](size_t i) {
        BOOST_CHECK(frame_changed_signum == 0);
        ++frame_changed_signum;

        BOOST_CHECK_EQUAL(i, 1);

        BOOST_REQUIRE_EQUAL(thrd.call_stack().size(), 2);

        auto frm1 = thrd.call_stack()[1];
        BOOST_REQUIRE(frm1);
        BOOST_REQUIRE(frm1->pos().src_pos().is_valid());
        BOOST_CHECK_EQUAL(frm1->pos().src_pos().line(), 21);
    });

    // updating thread
    {
        thread_info tinfo{200};
        tinfo.add_stack_frame({{100, 100, "main", {"test.cpp", 21}}, 1100, 0});
        thrd.update(tinfo);
    }

    BOOST_REQUIRE(thrd.call_stack().size() == 1);

    auto frm1 = thrd.call_stack()[0];
    BOOST_REQUIRE(frm1);
    BOOST_REQUIRE(frm1->pos().src_pos().is_valid());
    BOOST_CHECK_EQUAL(frm1->pos().src_pos().line(), 21);

    BOOST_CHECK_EQUAL(before_removed_signum, 1);
    BOOST_CHECK_EQUAL(after_removed_signum, 1);
    BOOST_CHECK_EQUAL(frame_changed_signum, 1);
}


/// Tests updating code position in current frame. For now, we remove and create
/// frame item again for such situation. This should be fixed in the future
/// (see CXXDBG-759 for details).
BOOST_AUTO_TEST_CASE(update_code_pos) {
    thread thrd{sources, 200};

    {
        thread_info tinfo{200};
        tinfo.add_stack_frame({{200, 200, "foo", {}}, 1000, 0});
        tinfo.add_stack_frame({{100, 100, "main", {}}, 1100, 0});
        thrd.update(tinfo);
    }

    int before_stack_frames_removed_called = 0;
    thrd.before_stack_frames_removed.connect([&before_stack_frames_removed_called, &thrd](size_t f, size_t l) {
        BOOST_CHECK_EQUAL(before_stack_frames_removed_called, 0);
        ++before_stack_frames_removed_called;

        BOOST_CHECK_EQUAL(f, 0);
        BOOST_CHECK_EQUAL(l, 0);

        BOOST_REQUIRE_EQUAL(thrd.call_stack().size(), 2);
        BOOST_CHECK((thrd.call_stack()[0]->pos() == code_position{200, 200, "foo"}));
        BOOST_CHECK((thrd.call_stack()[1]->pos() == code_position{100, 100, "main"}));
    });

    int after_stack_frames_removed_called = 0;
    thrd.after_stack_frames_removed.connect([&after_stack_frames_removed_called, &thrd](size_t f, size_t l) {
        BOOST_CHECK_EQUAL(after_stack_frames_removed_called, 0);
        ++after_stack_frames_removed_called;

        BOOST_CHECK_EQUAL(f, 0);
        BOOST_CHECK_EQUAL(l, 0);

        BOOST_REQUIRE_EQUAL(thrd.call_stack().size(), 1);
        BOOST_CHECK((thrd.call_stack()[0]->pos() == code_position{100, 100, "main"}));
    });

    int before_stack_frames_added_called = 0;
    thrd.before_stack_frames_added.connect([&before_stack_frames_added_called, &thrd](size_t f, size_t l) {
        BOOST_CHECK_EQUAL(before_stack_frames_added_called, 0);
        ++before_stack_frames_added_called;

        BOOST_CHECK_EQUAL(f, 0);
        BOOST_CHECK_EQUAL(l, 0);

        BOOST_REQUIRE_EQUAL(thrd.call_stack().size(), 1);
        BOOST_CHECK((thrd.call_stack()[0]->pos() == code_position{100, 100, "main"}));
    });

    int after_stack_frames_added_called = 0;
    thrd.after_stack_frames_added.connect([&after_stack_frames_added_called, &thrd](size_t f, size_t l) {
        BOOST_CHECK_EQUAL(after_stack_frames_added_called, 0);
        ++after_stack_frames_added_called;

        BOOST_CHECK_EQUAL(f, 0);
        BOOST_CHECK_EQUAL(l, 0);

        BOOST_REQUIRE_EQUAL(thrd.call_stack().size(), 2);
        BOOST_CHECK((thrd.call_stack()[0]->pos() == code_position{204, 204, "foo"}));
        BOOST_CHECK((thrd.call_stack()[1]->pos() == code_position{100, 100, "main"}));
    });

    thrd.stack_frame_changed.connect([](size_t i) {
        BOOST_CHECK(false);
    });

    // updating thread
    {
        thread_info tinfo{200};
        tinfo.add_stack_frame({{204, 204, "foo", {}}, 1000, 0});
        tinfo.add_stack_frame({{100, 100, "main", {}}, 1100, 0});
        thrd.update(tinfo);
    }

    BOOST_REQUIRE_EQUAL(thrd.call_stack().size(), 2);
    BOOST_CHECK((thrd.call_stack()[0]->pos() == code_position{204, 204, "foo"}));
    BOOST_CHECK((thrd.call_stack()[1]->pos() == code_position{100, 100, "main"}));

    BOOST_CHECK_EQUAL(before_stack_frames_removed_called, 1);
    BOOST_CHECK_EQUAL(after_stack_frames_removed_called, 1);
    BOOST_CHECK_EQUAL(before_stack_frames_added_called, 1);
    BOOST_CHECK_EQUAL(after_stack_frames_added_called, 1);
}


/// Tests updating code position in current frame. For now, we remove and create
/// frame item again for such situation. This should be fixed in the future
/// (see CXXDBG-759 for details).
/// This may happen when stepping inside the start function after returning from the main
/// function on Macos (see CXXDBG-752)
BOOST_AUTO_TEST_CASE(update_code_pos_single) {
    thread thrd{sources, 200};

    {
        thread_info tinfo{200};
        tinfo.add_stack_frame({{200, 200, "foo", {}}, 1000, 0});
        thrd.update(tinfo);
    }

    int before_stack_frames_removed_called = 0;
    thrd.before_stack_frames_removed.connect([&before_stack_frames_removed_called, &thrd](size_t f, size_t l) {
        BOOST_CHECK_EQUAL(before_stack_frames_removed_called, 0);
        ++before_stack_frames_removed_called;

        BOOST_CHECK_EQUAL(f, 0);
        BOOST_CHECK_EQUAL(l, 0);

        BOOST_REQUIRE_EQUAL(thrd.call_stack().size(), 1);
        BOOST_CHECK((thrd.call_stack()[0]->pos() == code_position{200, 200, "foo"}));
    });

    int after_stack_frames_removed_called = 0;
    thrd.after_stack_frames_removed.connect([&after_stack_frames_removed_called, &thrd](size_t f, size_t l) {
        BOOST_CHECK_EQUAL(after_stack_frames_removed_called, 0);
        ++after_stack_frames_removed_called;

        BOOST_CHECK_EQUAL(f, 0);
        BOOST_CHECK_EQUAL(l, 0);

        BOOST_REQUIRE_EQUAL(thrd.call_stack().size(), 0);
    });

    int before_stack_frames_added_called = 0;
    thrd.before_stack_frames_added.connect([&before_stack_frames_added_called, &thrd](size_t f, size_t l) {
        BOOST_CHECK_EQUAL(before_stack_frames_added_called, 0);
        ++before_stack_frames_added_called;

        BOOST_CHECK_EQUAL(f, 0);
        BOOST_CHECK_EQUAL(l, 0);

        BOOST_REQUIRE_EQUAL(thrd.call_stack().size(), 0);
    });

    int after_stack_frames_added_called = 0;
    thrd.after_stack_frames_added.connect([&after_stack_frames_added_called, &thrd](size_t f, size_t l) {
        BOOST_CHECK_EQUAL(after_stack_frames_added_called, 0);
        ++after_stack_frames_added_called;

        BOOST_CHECK_EQUAL(f, 0);
        BOOST_CHECK_EQUAL(l, 0);

        BOOST_REQUIRE_EQUAL(thrd.call_stack().size(), 1);
        BOOST_CHECK((thrd.call_stack()[0]->pos() == code_position{204, 204, "foo"}));
    });

    thrd.stack_frame_changed.connect([](size_t i) {
        BOOST_CHECK(false);
    });

    // updating thread
    {
        thread_info tinfo{200};
        tinfo.add_stack_frame({{204, 204, "foo", {}}, 1000, 0});
        thrd.update(tinfo);
    }

    BOOST_REQUIRE_EQUAL(thrd.call_stack().size(), 1);
    BOOST_CHECK((thrd.call_stack()[0]->pos() == code_position{204, 204, "foo"}));

    BOOST_CHECK_EQUAL(before_stack_frames_removed_called, 1);
    BOOST_CHECK_EQUAL(after_stack_frames_removed_called, 1);
    BOOST_CHECK_EQUAL(before_stack_frames_added_called, 1);
    BOOST_CHECK_EQUAL(after_stack_frames_added_called, 1);
}



BOOST_AUTO_TEST_SUITE_END()


}
