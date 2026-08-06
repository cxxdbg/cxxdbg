// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file simple_event_queue.cpp
/// Contains implementation of simple_event_queue class.

#include "simple_event_queue.hpp"
#include <iostream>


namespace cxxdbg { namespace async {


simple_event_queue::~simple_event_queue() {
}


void simple_event_queue::post(const std::function<void()> & func) {
    std::unique_lock<std::mutex> lock(mutex_);
    events_.push(func);
    cv_.notify_one();
}


std::function<void()> simple_event_queue::next_event() {
    std::unique_lock<std::mutex> lock(mutex_);

    // wating for events
    while (events_.empty()) {
        cv_.wait(lock);
    }

    // extractring first event
    assert(!events_.empty() && "invalid event queue state");
    std::function<void()> ev = events_.front();
    events_.pop();
    return ev;
}


void simple_event_queue::run() {
    while (true) {
        // extracting next event from core queue
        auto event = next_event();
        if (!event) {
            // quit event
            break;
        }

        // executing event handler
        event();
    }
}


bool simple_event_queue::empty() {
    std::unique_lock<std::mutex> lock(mutex_);
    return events_.empty();
}


} }
