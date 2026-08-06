// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file simple_event_queue.hpp
/// Contains definition of simple_event_queue class.

#pragma once

#include "event_queue.hpp"
#include <condition_variable>
#include <mutex>
#include <queue>


namespace cxxdbg { namespace async {


/// \class simple_event_queue
/// Simple thread safe implementation of event_queue abstract class
class simple_event_queue: public event_queue {
public:
    /// Destructor, destroys object
    virtual ~simple_event_queue();

    /// Puts event into queue
    virtual void post(const std::function<void()> & func);

    /// Extracts next event from queue. Blocks execution until
    /// new event received.
    std::function<void()> next_event();

    /// Process all events until empty event is reached
    void run();

    /// Checks if queue is empty
    bool empty();

private:
    /// Type of event queue
    typedef std::queue<std::function<void()>> event_queue;

    event_queue events_;            ///< Queue of events
    std::mutex mutex_;              ///< Mutex for sync between threads
    std::condition_variable cv_;    ///< Condition variable for signalling new events
};


} }


