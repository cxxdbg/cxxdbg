// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread.cpp
/// Contains implementation of thread class.

#include "thread.hpp"
#include "thread_info.hpp"
#include "stack_frame.hpp"
#include <ranges.hpp>
#include <algorithm>
#include <cassert>
#include <vector>


namespace cxxdbg::dbg {


constexpr thread::size_type thread::invalid_current_frame;


thread::thread(source_model & sources, unsigned long i):
sources_{sources},
id_{i},
has_complete_call_stack_{true},
current_frame_{invalid_current_frame} {
}


unsigned long thread::id() const {
    return id_;
}


std::size_t thread::call_stack_size() const {
    return call_stack_.size();
}


const stack_frame * thread::frame_at(std::size_t index) const {
    assert(index < call_stack_size() && "Invalid frame index");
    return call_stack_[index].get();
}


bool thread::has_complete_call_stack() const {
    return has_complete_call_stack_;
}


const code_position & thread::pos() const {
    assert(!call_stack_.empty() && "Call stack is empty");
    return call_stack_.front()->pos();
}


thread::size_type thread::current_frame_index() const {
    return current_frame_;
}


const stack_frame * thread::current_frame() const {
    auto idx = current_frame_index();
    if (idx == invalid_current_frame)
        return nullptr;

    return call_stack_.at(idx).get();
}


void thread::set_current_frame_index(size_type index) {
    assert(index < call_stack_.size() && "Invalid current frame index");
    auto old_idx = current_frame_;
    current_frame_ = index;

    if (old_idx != invalid_current_frame) {
        assert(old_idx < call_stack_.size() && "invalid old frame index");
        stack_frame_changed(old_idx);
    }

    if (current_frame_ != invalid_current_frame) {
        stack_frame_changed(current_frame_);
    }
}


void thread::set_current_frame(const stack_frame * frame) {
    auto cs = call_stack();
    auto it = std::ranges::find(cs, frame);
    assert(it != std::ranges::end(cs) && "Frame not found in thread");
    set_current_frame_index(std::ranges::distance(std::ranges::begin(cs), it));
}


/// Checks CFAs order in call stack in thread info
static bool check_cfa_order(const thread_info & tinfo) {
    uint64_t cfa = 0;
    for (const auto & finfo : tinfo.call_stack()) {
        if (cfa > finfo.cfa())
            return false;

        cfa = finfo.cfa();
    }

    return true;
}


/// < comapres CFAs of two stack frames
static bool cfa_le(const stack_frame & f1, const stack_frame & f2) {
    return f1.cfa() < f2.cfa();
}


/// == comapres two stack frames
static bool cfa_eq(const stack_frame & f1, const stack_frame & f2) {
    return f1.pos().file_addr() == f2.pos().file_addr() &&
           f1.pos().load_addr() == f2.pos().load_addr() &&
           f1.cfa() == f2.cfa() &&
           f1.sctx() == f2.sctx();
}


/// Compares stack frame and stack frame info
static bool stack_frame_eq(const stack_frame & f1, const stack_frame_info & f2) {
    return f1.pos().file_addr() == f2.pos().file_addr() &&
           f1.pos().load_addr() == f2.pos().load_addr() &&
           f1.cfa() == f2.cfa() &&
           f1.sctx() == f2.sc_id();
}


void thread::update(const thread_info & tinfo) {

    // CFAs should be sorted in call stack
    bool ordered = check_cfa_order(tinfo);

    set_has_complete_call_stack(tinfo.has_complete_call_stack());

    if (!cfa_ordered_ || !ordered) {
        // can't update call stack with bad CFA order,
        // removing all
        size_t nframes = call_stack_.size();
        before_stack_frames_removed(0, nframes - 1);
        call_stack_.clear();
        after_stack_frames_removed(0, nframes - 1);
    }

    cfa_ordered_ = ordered;

    if (call_stack_.empty()) {
        // updating empty call stack

        if (tinfo.call_stack_size() == 0) {
            // doing nothing
            return;
        }

        size_t nframes = tinfo.call_stack_size();
        before_stack_frames_added(0, nframes - 1);

        for (const auto & finfo : tinfo.call_stack()) {
            call_stack_.push_back(make_frame(finfo));
        }

        after_stack_frames_added(0, nframes - 1);
        return;
    }

    if (tinfo.call_stack().empty()) {
        assert(!call_stack_.empty() && "call stack should not be empty here");

        size_t nframes = call_stack_.size();
        before_stack_frames_removed(0, nframes - 1);
        call_stack_.clear();
        current_frame_ = invalid_current_frame;
        after_stack_frames_removed(0, nframes - 1);
        return;
    }

    assert(!call_stack_.empty() && "call stack should not be empty here");
    assert(!tinfo.call_stack().empty() && "tinfo call stack should not be empty here");

    // looking for last CFA in new call stack
    auto it = std::upper_bound(tinfo.call_stack().begin(),
                               tinfo.call_stack().end(),
                               call_stack_.back()->cfa(), [](const auto & cfa, const auto & sf) {
        return cfa < sf.cfa();
    });

    if (// the last CFA in the current call stack is less than the first CFA in the new call stack
        it == tinfo.call_stack().begin() ||

        // the last CFA in the current call stack is equal to the last CFA in the new call stack,
        // but stack frames are not equal (see CXXDBG-752 and CXXDBG-759)
        it == tinfo.call_stack().end() &&
        call_stack_.back()->cfa() == tinfo.call_stack().back().cfa() &&
        !stack_frame_eq(*call_stack_.back(), tinfo.call_stack().back())) {

        // no overlap
        remove_end(call_stack_.begin());
        insert_end(tinfo.call_stack(), tinfo.call_stack().begin());
        return;
    }

    if (it != tinfo.call_stack().end() ||
        stack_frame_eq(*call_stack_.back(), tinfo.call_stack().back())) {

        auto new_last = it;
        auto old_last = call_stack_.end();

        auto new_first = std::prev(new_last);
        auto old_first = std::prev(old_last);

        // comparing call stack from last stack frame
        while (true) {
            // checking for begin of stack
            if (new_first == tinfo.call_stack().begin() ||
                old_first == call_stack_.begin())
                break;

            auto p_new_begin = std::prev(new_first);
            auto p_old_begin = std::prev(old_first);

            if (!stack_frame_eq(**p_old_begin, *p_new_begin))
                break;

            new_first = p_new_begin;
            old_first = p_old_begin;
        }

        update_stack_frames(old_first, old_last, new_first);

        // [new_first, new_last) == [old_first, old_last)
        remove_begin(old_first);
        remove_end(old_last);

        insert_begin(tinfo.call_stack(), new_first);
        insert_end(tinfo.call_stack(), new_last);

        return;
    }

    // looking for last CFA from new call stack in old call stack
    auto it2 = std::upper_bound(call_stack_.begin(),
                                call_stack_.end(),
                                tinfo.call_stack().back().cfa(), [](const auto & cfa, const auto & sf) {
        return cfa < sf->cfa();
    });


    if (it2 == call_stack_.begin()) {
        // no overlap
        remove_end(call_stack_.begin());
        insert_end(tinfo.call_stack(), tinfo.call_stack().begin());
        return;
    }

    // should be handled in first no-overlap check
    assert(it2 != call_stack_.end() && "invalid search result");

    auto new_last = tinfo.call_stack().end();
    auto old_last = it2;

    auto new_first = std::prev(new_last);
    auto old_first = std::prev(old_last);

    // comparing call stack from last stack frame
    while (true) {
        // checking for begin of stack
        if (new_first == tinfo.call_stack().begin() ||
            old_first == call_stack_.begin())
            break;

        auto p_new_begin = std::prev(new_first);
        auto p_old_begin = std::prev(old_first);

        if (!stack_frame_eq(**p_old_begin, *p_new_begin))
            break;

        new_first = p_new_begin;
        old_first = p_old_begin;
    }

    update_stack_frames(old_first, old_last, new_first);

    // [new_first, new_last) == [old_first, old_last)
    remove_begin(old_first);
    remove_end(old_last);

    insert_begin(tinfo.call_stack(), new_first);
    insert_end(tinfo.call_stack(), new_last);
}


thread::stack_frame_ptr thread::make_frame(const stack_frame_info & info) {
    // making stack frame
    code_position pos(info.pos(), sources_);
    std::shared_ptr<stack_frame> frame{new stack_frame{pos, info.cfa(), info.sc_id()}};

    // adding parameters to stack frame
    for (const auto & pinfo : info.params()) {
        frame->add_param(pinfo);
    }

    return frame;
}


void thread::remove_begin(const stack_frame_deque::iterator & last) {
    if (last == call_stack_.begin())
        return;

    auto count = std::distance(call_stack_.begin(), last);
    assert(count > 0 && "invalid difference");
    auto scount = static_cast<size_t>(count);

    before_stack_frames_removed(0, scount - 1);
    call_stack_.erase(call_stack_.begin(), last);

    if (current_frame_ != invalid_current_frame &&
        current_frame_ < scount) {
        current_frame_ = invalid_current_frame;
    } else {
        current_frame_ -= scount;
    }

    after_stack_frames_removed(0, scount - 1);
}


void thread::remove_end(const stack_frame_deque::iterator & first) {
    if (first == call_stack_.end())
        return;

    auto first_idx = std::distance(call_stack_.begin(), first);
    assert(first_idx >= 0 && "invalid distance");
    auto sfirst_idx = static_cast<size_t>(first_idx);
    auto sz = call_stack_.size();
    assert(sz > 0 && "invalid call stack size");

    before_stack_frames_removed(sfirst_idx, sz - 1);
    call_stack_.erase(first, call_stack_.end());

    if (current_frame_ != invalid_current_frame &&
        current_frame_ >= sfirst_idx) {
        current_frame_ = invalid_current_frame;
    }

    after_stack_frames_removed(sfirst_idx, sz - 1);
}


void thread::insert_begin(const std::vector<stack_frame_info> & frames,
                          const std::vector<stack_frame_info>::const_iterator & last) {
    if (last == frames.begin())
        return;

    auto sz = std::distance(frames.begin(), last);
    auto ssz = static_cast<size_t>(sz);
    assert(ssz > 0 && "invalid distance");

    before_stack_frames_added(0, ssz - 1);

    {
        auto it = last;
        auto first = frames.begin();
        do {
            --it;
            call_stack_.push_front(make_frame(*it));
        } while (it != first);
    }

    if (current_frame_ != invalid_current_frame) {
        current_frame_ += ssz;
    }

    after_stack_frames_added(0, ssz - 1);
}


void thread::insert_end(const std::vector<stack_frame_info> & frames,
                        const std::vector<stack_frame_info>::const_iterator & first) {
    if (first == frames.end())
        return;

    auto start_idx = call_stack_.size();
    auto sz = std::distance(first, frames.end());
    auto ssz = static_cast<size_t>(sz);
    assert(ssz > 0 && "invalid distance");

    before_stack_frames_added(start_idx, start_idx + ssz - 1);

    for (auto it = first, last = frames.end(); it != last; ++it) {
        call_stack_.push_back(make_frame(*it));
    }

    after_stack_frames_added(start_idx, start_idx + ssz - 1);
}


void thread::update_stack_frame(const stack_frame_deque::iterator it,
                                const stack_frame_info & info) {

    stack_frame & frm = **it;
    bool changed = false;

    if (frm.params().size() != info.params().size()) {
        frm.params().clear();
        frm.params().reserve(info.params().size());
        for (const auto & pinfo : info.params()) {
            frm.add_param(pinfo);
        }

        changed = true;
    } else {

        for (size_t i = 0, sz = frm.params().size(); i < sz; ++i) {
            call_parameter & par = frm.params()[i];
            const call_parameter_info & pinfo = info.params()[i];

            if (par.name() != pinfo.name() ||
                par.value() != pinfo.value() ||
                par.type() != pinfo.type()) {

                changed = true;
                frm.params()[i] = {pinfo.name(),
                                   pinfo.type(),
                                   pinfo.value()};
            }
        }
    }

    // updating source position if different
    // (may happen after returning from function)

    if (info.pos().src_pos().is_valid()) {
        source_position new_src_pos{info.pos().src_pos(), sources_};

        if (frm.pos().src_pos() != new_src_pos) {
            frm.pos().set_src_pos(new_src_pos);
            changed = true;
        }
    } else {
        if (frm.pos().src_pos().is_valid()) {
            frm.pos().set_src_pos({});
            changed = true;
        }
    }


    if (changed) {
        size_t idx = static_cast<size_t>(std::distance(call_stack_.begin(), it));
        stack_frame_changed(idx);
    }
}


void thread::update_stack_frames(const stack_frame_deque::iterator & first,
                                 const stack_frame_deque::iterator & last,
                                 const std::vector<stack_frame_info>::const_iterator & ifirst) {

    auto it = first;
    auto iit = ifirst;

    while (it != last) {
        update_stack_frame(it, *iit);
        ++it;
        ++iit;
    }
}


void thread::set_has_complete_call_stack(bool v) {
    if (has_complete_call_stack_ == v)
        return;

    before_complete_call_stack_changed();
    has_complete_call_stack_ = v;
    after_complete_call_stack_changed();
}


}
