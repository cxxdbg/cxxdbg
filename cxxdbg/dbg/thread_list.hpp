// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread_list.hpp
/// Contains definition of thread_list class.

#pragma once

#include "forward.hpp"
#include "stack_frame.hpp"
#include "thread.hpp"
#include "thread_list_info.hpp"
#include "cxxdbg/app/signals.hpp"

#include <ranges.hpp>
#include <memory>
#include <vector>
#include <boost/iterator/iterator_adaptor.hpp>


namespace cxxdbg::dbg {


class thread;


/// \class thread_list
/// Represents thread list in process being debugged
class thread_list {

    /// Type of shared pointer to thread
    typedef std::shared_ptr<thread> thread_ptr;

    /// Type of vector of threads
    typedef std::vector<thread_ptr> thread_vector;

    /// Returns range of threads from vector of shared pointer to thread objects
    static auto make_threads_range(const thread_vector & tv) {
        auto fn = [](auto && ptr) { return static_cast<const thread*>(ptr.get()); };
        return tv | std::ranges::views::transform(fn);
    }

public:
    /// Type of const iterator over list of threads
    class const_iterator;

    /// Type of size of thread list
    typedef thread_vector::size_type size_type;


    /// Constructor, makes empty thread list with specified pointer to source model
    thread_list(source_model & src_mdl);

    /// Returns const iterator pointing to the first thread in list
    const_iterator begin() const;

    /// Returns const iterator pointing to thchanged_callede one past last thread in list
    const_iterator end() const;

    /// Returns number of threads in list
    size_type size() const;

    /// Returns pointer to thread at specified index
    thread * at(size_type i);

    /// Returns const pointer to thread at specified index
    const thread * at(size_type i) const;

    /// Returns pointer to active thread
    thread * current_thread();

    /// Returns const pointer to active thread
    const thread * current_thread() const;

    /// Sets active thread
    void set_current_thread(const thread * thrd);

    /// Updates thread list according to specified thread list info
    void update(const thread_list_info & tlinfo);

    /// Updates current thread
    void update_current(const thread_info & tinfo);

    /// Clears thread list
    void clear();

    /// Current thread changed signal. Emited after current
    /// thread changed. Old thread object still exists at this point
    CXXDBG_DEFINE_SIGNALX(current_thread_changed, void());


    /// Returns const range of all threads
    auto all_threads() const {
        return make_threads_range(threads_);
    }

    /// Returns empty range of threads. Helper function to make empty range of threads
    /// of the same type as all_threads() returns
    static auto empty_threads() {
        return make_threads_range(empty_threads_);
    }

    /// Returns range of threads located at specified location
    auto at_line(const source_position & pos) const {
        auto fn = [pos](auto && thrd) {
            if (std::ranges::empty(thrd->call_stack())) {
                // no call stack info in thread
                return false;
            }

            const stack_frame * frame = thrd->call_stack().front();

            if (!frame->pos().src_pos()) {
                // no source position is available
                return false;
            }

            if (frame->pos().src_pos() != pos) {
                return false;
            }

            return true;
        };

        return all_threads() | std::ranges::views::filter(fn);
    }

private:
    static constexpr size_type invalid_thread_index =
            std::numeric_limits<size_type>::max();

    /// Returns index of active thread
    auto current_thread_index() const { return current_thread_; }

    /// Sets index of active thread
    void set_current_thread(size_type index);

    source_model & src_model_;      ///< Reference to source model
    thread_vector threads_;         ///< List of threads
    size_type current_thread_;      ///< Index of active thread

    ///< Empty list of threads. Helper variable for the empty_threads static function
    static thread_vector empty_threads_;
};


/// \class thread_list::const_iterator
/// Const iterator over threads in thread list
class thread_list::const_iterator: public boost::iterator_adaptor <
    const_iterator,
    std::vector<std::shared_ptr<thread>>::const_iterator,
    const thread *,
    boost::use_default,
    const thread *
> {
public:
    /// Constructor, makes iterator pointing to thread pointed by
    /// specified vector iterator
    explicit const_iterator(const base_type & it = base_type()):
        iterator_adaptor_(it) {}

    /// Dereferences iterator
    reference dereference() const {
        return base()->get();
    }
};


}


