// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread_test.cpp
/// Contains unit tests for the thread_list class.

#include "../source_model.hpp"
#include "../thread.hpp"
#include "../thread_list.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::test {


struct thread_list_test_fixture {
    source_model sources;
    thread_list threads{sources};
};


BOOST_FIXTURE_TEST_SUITE(thread_list_test, thread_list_test_fixture)


/// Tests thread list constructor
BOOST_AUTO_TEST_CASE(ctor) {
    BOOST_CHECK(threads.begin() == threads.end());
    BOOST_CHECK_EQUAL(threads.size(), 0);
    BOOST_CHECK(threads.current_thread() == nullptr);
}


/// Tests adding thread to thread list
BOOST_AUTO_TEST_CASE(add_thread) {
    thread_info tinfo{200};
    tinfo.add_stack_frame({{100, 100, "func", {}}, 4000, 0});
    tinfo.set_has_complete_call_stack(true);

    thread_list_info linfo;
    linfo.add(tinfo);
    linfo.set_current_thread(0);

    threads.update(linfo);

    BOOST_CHECK(threads.begin() != threads.end());
    BOOST_CHECK_EQUAL(threads.size(), 1);
    BOOST_CHECK_EQUAL(threads.current_thread(), threads.at(0));

    auto thrd = threads.at(0);
    BOOST_REQUIRE(thrd);
    BOOST_CHECK_EQUAL(thrd->id(), 200);
}


/// Tests adding second thread to thread list via update
BOOST_AUTO_TEST_CASE(add_second_thread) {
    // add first thread
    thread_info tinfo{200};
    tinfo.add_stack_frame({{100, 100, "func", {}}, 4000, 0});
    tinfo.set_has_complete_call_stack(true);

    {
        thread_list_info linfo;
        linfo.add(tinfo);
        linfo.set_current_thread(0);
        threads.update(linfo);
    }

    BOOST_REQUIRE_EQUAL(threads.size(), 1);
    auto thrd1 = threads.at(0);
    BOOST_REQUIRE(thrd1 != nullptr);
    auto t1id = thrd1->id();


    // adding second thread
    thread_info tinfo2{500};
    tinfo2.add_stack_frame({{200, 200, "foo", {}}, 4000, 0});
    tinfo2.set_has_complete_call_stack(true);

    {
        thread_list_info linfo;
        linfo.add(tinfo2);
        linfo.add(tinfo);
        linfo.set_current_thread(1);
        threads.update(linfo);
    }

    // thread 1 should be same
    BOOST_REQUIRE_EQUAL(threads.size(), 2);
    auto new_thrd1 = threads.at(1);
    BOOST_CHECK_EQUAL(thrd1, new_thrd1);
    BOOST_CHECK_EQUAL(t1id, new_thrd1->id());

    BOOST_CHECK(threads.at(0)->id() == 500);
}


/// Tests removing threads from thread list via update
BOOST_AUTO_TEST_CASE(remove_threads) {
    // add threads
    thread_info tinfo{200};
    tinfo.add_stack_frame({{100, 100, "func", {}}, 4000, 0});
    tinfo.set_has_complete_call_stack(true);

    thread_info tinfo2{500};
    tinfo2.add_stack_frame({{200, 200, "foo", {}}, 4000, 0});
    tinfo2.set_has_complete_call_stack(true);

    thread_info tinfo3{600};
    tinfo3.add_stack_frame({{200, 200, "aaaa", {}}, 4000, 0});
    tinfo3.set_has_complete_call_stack(true);

    {
        thread_list_info linfo;
        linfo.add(tinfo);
        linfo.add(tinfo2);
        linfo.add(tinfo3);
        linfo.set_current_thread(1);
        threads.update(linfo);
    }

    BOOST_REQUIRE_EQUAL(threads.size(), 3);
    auto thrd2_orig = threads.at(1);
    BOOST_REQUIRE(thrd2_orig != nullptr);
    auto t2id = thrd2_orig->id();

    BOOST_CHECK_EQUAL(threads.at(0)->id(), 200);
    BOOST_CHECK_EQUAL(threads.at(1)->id(), 500);
    BOOST_CHECK_EQUAL(threads.at(2)->id(), 600);


    // removing thread 1 and 3
    {
        thread_list_info linfo;
        linfo.add(tinfo2);
        linfo.set_current_thread(0);
        threads.update(linfo);
    }

    // thread 2 should be same
    BOOST_REQUIRE_EQUAL(threads.size(), 1);
    auto new_thrd2 = threads.at(0);
    BOOST_CHECK_EQUAL(thrd2_orig, new_thrd2);
    BOOST_CHECK_EQUAL(t2id, new_thrd2->id());
}


/// Tests adding and removing threads via update
BOOST_AUTO_TEST_CASE(add_remove_threads) {
    // add 2 threads
    thread_info tinfo{200};
    tinfo.add_stack_frame({{100, 100, "func", {}}, 4000, 0});
    tinfo.set_has_complete_call_stack(true);

    thread_info tinfo2{500};
    tinfo2.add_stack_frame({{200, 200, "foo", {}}, 4000, 0});
    tinfo2.set_has_complete_call_stack(true);

    thread_info tinfo3{600};
    tinfo3.add_stack_frame({{200, 200, "aaaa", {}}, 4000, 0});
    tinfo3.set_has_complete_call_stack(true);

    // add thread 1 and 2
    {
        thread_list_info linfo;
        linfo.add(tinfo);
        linfo.add(tinfo2);
        linfo.set_current_thread(1);
        threads.update(linfo);
    }

    BOOST_REQUIRE_EQUAL(threads.size(), 2);
    auto thrd2_orig = threads.at(1);
    BOOST_REQUIRE(thrd2_orig != nullptr);
    auto t2id = thrd2_orig->id();


    // removing thread 1 and adding thread 3
    {
        thread_list_info linfo;
        linfo.add(tinfo2);
        linfo.add(tinfo3);
        linfo.set_current_thread(0);
        threads.update(linfo);
    }

    // thread 2 should be same
    BOOST_REQUIRE_EQUAL(threads.size(), 2);
    auto new_thrd2 = threads.at(0);
    BOOST_CHECK_EQUAL(thrd2_orig, new_thrd2);
    BOOST_CHECK_EQUAL(t2id, new_thrd2->id());

    BOOST_CHECK(threads.at(0)->id() == 500);
    BOOST_CHECK(threads.at(1)->id() == 600);
}


/// Tests changing current thread after update
BOOST_AUTO_TEST_CASE(change_current) {
    // add 2 threads
    thread_info tinfo{200};
    tinfo.add_stack_frame({{100, 100, "func", {}}, 4000, 0});
    tinfo.set_has_complete_call_stack(true);

    thread_info tinfo2{500};
    tinfo2.add_stack_frame({{200, 200, "foo", {}}, 4000, 0});
    tinfo2.set_has_complete_call_stack(true);

    thread_info tinfo3{600};
    tinfo3.add_stack_frame({{200, 200, "aaaa", {}}, 4000, 0});
    tinfo3.set_has_complete_call_stack(true);

    // add thread 1 and 2
    {
        thread_list_info linfo;
        linfo.add(tinfo);
        linfo.add(tinfo2);
        linfo.set_current_thread(1);
        threads.update(linfo);
    }

    BOOST_REQUIRE_EQUAL(threads.size(), 2);
    BOOST_CHECK_EQUAL(threads.at(0)->id(), 200);
    BOOST_CHECK_EQUAL(threads.at(1)->id(), 500);
    BOOST_REQUIRE(threads.current_thread());
    BOOST_CHECK_EQUAL(threads.current_thread()->id(), 500);

    auto thrd2 = threads.at(1);

    unsigned int sig_num = 0;

    threads.current_thread_changed().connect([this, thrd2, &sig_num] {

        if (sig_num == 0) {
            // current thread changed should be emited after setting
            // current thread to invalid

            // thread 2 object should be valid here
            BOOST_CHECK_EQUAL(thrd2->id(), 500);
            bool found = false;
            for (const auto * thrd : threads) {
                if (thrd == thrd2) {
                    found = true;
                    break;
                }
            }

            BOOST_CHECK(threads.current_thread() == nullptr);

        } else if (sig_num == 1) {
            // current thread changed should be emited after setting
            // new current thread
            BOOST_REQUIRE(threads.current_thread());
            BOOST_CHECK_EQUAL(threads.current_thread()->id(), 600);
        } else {
            BOOST_CHECK(false);
        }

        ++sig_num;
    });


    // removing thread 2 and adding thread 3
    {
        thread_list_info linfo;
        linfo.add(tinfo);
        linfo.add(tinfo3);
        linfo.set_current_thread(1);
        threads.update(linfo);
    }

    // thread 2 should be same
    BOOST_REQUIRE_EQUAL(threads.size(), 2);

    BOOST_CHECK_EQUAL(threads.at(0)->id(), 200);
    BOOST_CHECK_EQUAL(threads.at(1)->id(), 600);
    BOOST_REQUIRE(threads.current_thread());
    BOOST_CHECK(threads.at(1) == threads.current_thread());
    BOOST_CHECK_EQUAL(sig_num, 2);
}


/// Tests changing current thread after update
/// without deleting current thread
BOOST_AUTO_TEST_CASE(change_current_nodelete) {
    thread_info tinfo{200};
    tinfo.add_stack_frame({{100, 100, "func", {}}, 4000, 0});
    tinfo.set_has_complete_call_stack(true);

    thread_info tinfo2{500};
    tinfo2.add_stack_frame({{200, 200, "foo", {}}, 4000, 0});
    tinfo2.set_has_complete_call_stack(true);

    thread_info tinfo3{600};
    tinfo3.add_stack_frame({{200, 200, "aaaa", {}}, 4000, 0});
    tinfo3.set_has_complete_call_stack(true);

    // add thread 1 and 2
    {
        thread_list_info linfo;
        linfo.add(tinfo);
        linfo.add(tinfo2);
        linfo.set_current_thread(1);
        threads.update(linfo);
    }

    BOOST_REQUIRE_EQUAL(threads.size(), 2);
    BOOST_CHECK_EQUAL(threads.at(0)->id(), 200);
    BOOST_CHECK_EQUAL(threads.at(1)->id(), 500);
    BOOST_REQUIRE(threads.current_thread());
    BOOST_CHECK_EQUAL(threads.current_thread()->id(), 500);

    auto thrd2 = threads.at(1);

    bool changed_called = false;

    threads.current_thread_changed().connect([this, thrd2, &changed_called] {

        // current thread changed should be emited after setting
        // new current thread

        BOOST_CHECK(!changed_called);
        BOOST_REQUIRE(threads.current_thread());
        BOOST_CHECK_EQUAL(threads.current_thread()->id(), 600);
        changed_called = true;
    });


    // removing thread 1 and adding thread 3
    {
        thread_list_info linfo;
        linfo.add(tinfo2);
        linfo.add(tinfo3);
        linfo.set_current_thread(1);
        threads.update(linfo);
    }

    // thread 2 should be same
    BOOST_REQUIRE_EQUAL(threads.size(), 2);

    BOOST_CHECK_EQUAL(threads.at(0)->id(), 500);
    BOOST_CHECK_EQUAL(threads.at(1)->id(), 600);
    BOOST_REQUIRE(threads.current_thread());
    BOOST_CHECK(threads.at(1) == threads.current_thread());
    BOOST_CHECK(changed_called);
}



BOOST_AUTO_TEST_SUITE_END()


}
