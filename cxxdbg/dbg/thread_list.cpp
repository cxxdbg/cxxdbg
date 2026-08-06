// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread_list.cpp
/// Contains implementation of thread_list class.

#include "thread_list.hpp"
#include "code_position.hpp"
#include "source_model.hpp"
#include "source_position.hpp"
#include "stack_frame.hpp"
#include "thread.hpp"
#include <algorithm>
#include <cassert>


namespace cxxdbg::dbg {


thread_list::thread_vector thread_list::empty_threads_;


thread_list::thread_list(source_model & src_mdl):
src_model_(src_mdl),
current_thread_(invalid_thread_index) {
}


thread_list::const_iterator thread_list::begin() const {
    return const_iterator(threads_.begin());
}


thread_list::const_iterator thread_list::end() const {
    return const_iterator(threads_.end());
}


thread_list::size_type thread_list::size() const {
    return threads_.size();
}


thread * thread_list::at(size_type i) {
    return threads_.at(i).get();
}


const thread *thread_list::at(size_type i) const {
    return threads_.at(i).get();
}


thread * thread_list::current_thread() {
    if (current_thread_index() == invalid_thread_index)
        return nullptr;

    return at(current_thread_index());
}


const thread * thread_list::current_thread() const {
    if (current_thread_index() == invalid_thread_index)
        return nullptr;

    return at(current_thread_index());
}


void thread_list::set_current_thread(const thread * thrd) {
    if (thrd == nullptr) {
        set_current_thread(invalid_thread_index);
        return;
    }

    const_iterator it = std::find(begin(), end(), thrd);
    assert(it != end() && "Thread not found in thread list");

    set_current_thread(std::distance(begin(), it));
}


void thread_list::update(const thread_list_info & tlinfo) {

    // building map of current threads with ids
    std::map<unsigned long, std::shared_ptr<thread>> curr_threads;
    for (const auto & thrd : threads_) {
        curr_threads.insert(std::make_pair(thrd->id(), thrd));
    }

    // if current thread is not in the new list of threads then
    // set current thread to undefined
    if (current_thread() != nullptr) {
        auto it = std::find_if(tlinfo.begin(), tlinfo.end(),
        [id = current_thread()->id()](auto && thrd) {
            return thrd.id() == id;
        });

        if (it == tlinfo.end()) {
            set_current_thread(nullptr);
        }
    }


    // removing all threads from vector
    threads_.clear();

    // building new list of threads using map of current threads
    // to reuse thread objects
    for (auto tit = tlinfo.begin(), tend = tlinfo.end(); tit != tend; ++tit) {
        std::shared_ptr<thread> thrd;

        auto curr_thread_it = curr_threads.find(tit->id());
        if (curr_thread_it != curr_threads.end()) {
            // old thread
            thrd = curr_thread_it->second;
        } else {
            // new thread
            thrd = std::make_shared<thread>(src_model_, tit->id());
        }

        thrd->update(*tit);
        threads_.push_back(thrd);

        if (!threads_.back()->call_stack().empty()) {
            threads_.back()->set_current_frame_index(tit->curr_frame_index());
        }
    }

    // set current thread
    set_current_thread(tlinfo.current_thread());
}


void thread_list::update_current(const thread_info & tinfo) {
    current_thread()->update(tinfo);
}


void thread_list::clear() {
    // setting current thread to invalid
    set_current_thread(nullptr);

    // clearing list of threads
    threads_.clear();
}


void thread_list::set_current_thread(size_type index) {
    assert((index == invalid_thread_index || index < size())
           && "Invalid index of active thread");
    current_thread_ = index;

    current_thread_changed()();
}


}
