// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread.hpp
/// Contains definition of thread class.

#pragma once

#include "cxxdbg/app/signals.hpp"
#include "cxxdbg/util/range_utils.hpp"
#include <memory>
#include <deque>


namespace cxxdbg::dbg {


class code_position;
class source_model;
class stack_frame;
class stack_frame_info;
class thread_info;


/// \class thread
/// Represents a thread in process being debugged
class thread {
    /// Type of shared pointer to stack frame
    typedef std::shared_ptr<stack_frame> stack_frame_ptr;

    /// Type of vector of stack frames
    typedef std::deque<stack_frame_ptr> stack_frame_deque;

    /// Call stack range transformer
    struct call_stack_range_transformer;

public:
    /// Type of size of call stack
    typedef stack_frame_deque::size_type size_type;

    /// Invalid frame index
    static constexpr stack_frame_deque::size_type invalid_current_frame =
        std::numeric_limits<stack_frame_deque::size_type>::max();

    /// Constructor, makes thread with specified reference to source model
    /// and thread ID
    thread(source_model & src_m, unsigned long i);

    /// Returns thread id
    unsigned long id() const;

    /// Returns number of frames incall stack
    std::size_t call_stack_size() const;

    /// Returns point to stack frame with secified index
    const stack_frame * frame_at(std::size_t index) const;

    /// Returns const call stack range
    const auto & call_stack() const { return call_stack_r_; }

    /// Returns const reference to call stack deque
    auto & call_stack_raw() const {
        return call_stack_;
    }

    /// Returns true if thread has complete call stack
    bool has_complete_call_stack() const;

    /// Returns position of first stack frame in thread
    const code_position & pos() const;

    /// Returns current frame index
    size_type current_frame_index() const;

    /// Returns const pointer to current frame
    const stack_frame * current_frame() const;

    /// Sets current frame index
    void set_current_frame_index(size_type index);

    /// Sets current frame
    void set_current_frame(const stack_frame * frame);

    /// Updates thread with specified thread info
    void update(const thread_info & tinfo);

    /// Before stack frames removed signal. The signal is emitted before
    /// stack frames removed from thread.
    mutable signal<void (size_t, size_t)> before_stack_frames_removed;

    /// After stack frames removed signal. The signal is emitted after
    /// stack frames removed from thread.
    mutable signal<void (size_t, size_t)> after_stack_frames_removed;

    /// Before stack frames added signal. The signal is emitted before
    /// stack frames added to thread.
    mutable signal<void (size_t, size_t)> before_stack_frames_added;

    /// After stack frames added signal. The signal is emitted after
    /// stack frames added to thread.
    mutable signal<void (size_t, size_t)> after_stack_frames_added;

    /// Stack frame changed signal. The signal is emitted after stack frame
    /// changed.
    mutable signal<void (size_t)> stack_frame_changed;

    /// Before complete call stack changed signal. The signal is emitted before
    /// complete call stack flag changed.
    mutable signal<void ()> before_complete_call_stack_changed;

    /// After complete call stack changed signal. The signal is emitted after
    /// complete call stack flag changed.
    mutable signal<void ()> after_complete_call_stack_changed;

private:
    /// Creates stack frame from stack frame info
    stack_frame_ptr make_frame(const stack_frame_info & info);

    /// Removes stack frames from begining of call stack
    void remove_begin(const stack_frame_deque::iterator & last);

    /// Removes stack frames from end of call stack
    void remove_end(const stack_frame_deque::iterator & first);

    /// Inserts stack frames at begining of call stack
    void insert_begin(const std::vector<stack_frame_info> & frames,
                      const std::vector<stack_frame_info>::const_iterator & last);

    /// Inserts stack frames after end of call stack
    void insert_end(const std::vector<stack_frame_info> & frames,
                    const std::vector<stack_frame_info>::const_iterator & first);

    /// Updates stack frame
    void update_stack_frame(const stack_frame_deque::iterator it,
                            const stack_frame_info & info);

    /// Updates stack frames
    void update_stack_frames(const stack_frame_deque::iterator & first,
                             const stack_frame_deque::iterator & last,
                             const std::vector<stack_frame_info>::const_iterator & ifirst);

    /// Sets complete call stack flag
    void set_has_complete_call_stack(bool v);


    source_model & sources_;                        ///< Reference to source model
    unsigned long id_;                              ///< Thread id
    stack_frame_deque call_stack_;                  ///< Thread call stack
    bool has_complete_call_stack_;                  ///< True if thread has complete call stack
    stack_frame_deque::size_type current_frame_;    ///< Current frame index for thread
    bool cfa_ordered_ = true;                       ///< True if stack frames ordered by CFA

    /// Range of const pointers to stack frames
    decltype(util::make_const_ptr_range(call_stack_)) call_stack_r_ = util::make_const_ptr_range(call_stack_);
};


}


