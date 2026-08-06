// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file event_queue.hpp
/// Contains definition of event_queue class

#pragma once

#include <functional>
#include <memory>
#include <boost/asio/execution_context.hpp>


namespace cxxdbg { namespace async {


class event_queue_executor;


/// \class event_queue
/// Represents an abstract event queue with single method for
/// putting events into the queue.
class event_queue: public boost::asio::execution_context {
public:
    /// Type of executor for queue
    using executor_type = event_queue_executor;

    /// Destructor, destroys object
    virtual ~event_queue() {}

    /// Puts event into queue
    virtual void post(const std::function<void()> & func) = 0;

    /// Returns executor for this queue
    executor_type get_executor();
};


class event_queue_executor {
    /// Function wrapper that is used to call functional objects passed to post functions.
    /// We have to store functional obejcts as shared pointers because event_queue::post
    /// takes std::function as a parameter, but ASIO executor concept requires that
    /// functor objects are not necessary copyable (but movable)
    template <typename F>
    class func_wrapper {
    public:
        /// Constructs wrapper with rvalue reference to functor
        func_wrapper(F && f):
            f_{std::make_shared<F>(std::move(f))} {}

        /// Executes function wrapper
        void operator()() const {
            (*f_)();
        }

    private:
        std::shared_ptr<F> f_;              ///< Stored function
    };

public:
    /// Constructs executor for specified event queue
    event_queue_executor(event_queue & q):
        q_{q} {}

    /// Copy constructor
    event_queue_executor(const event_queue_executor & exec) = default;

    /// Move constructor
    event_queue_executor(event_queue_executor && exec) = default;

    /// Returns reference to execution context (event queue)
    event_queue & context() const { return q_; }

    /// Does nothing
    void on_work_started() {}

    /// Does nothing
    void on_work_finished() {}

    /// Posts event to queue
    template <typename Func, typename Allocator>
    void post(Func && f, const Allocator & alloc) {
        q_.post(func_wrapper{std::forward<Func>(f)});
    }

    /// Posts event to queue
    template <typename Func, typename Allocator>
    void defer(Func && f, const Allocator & alloc) {
        post(std::forward<Func>(f), alloc);
    }

    /// Posts event to queue
    template <typename Func, typename Allocator>
    void dispatch(Func && f, const Allocator & alloc) {
        post(std::forward<Func>(f), alloc);
    }

    /// Compares this executor with another
    bool operator==(const event_queue_executor & exec) const {
        return &q_ == &exec.q_;
    }

    /// Compares this executor with another
    bool operator!=(const event_queue_executor & exec) const {
        return !(*this == exec);
    }

private:
    event_queue & q_;           ///< Reference to event queue
};


inline event_queue_executor event_queue::get_executor() {
    return event_queue_executor{*this};
}


} }


