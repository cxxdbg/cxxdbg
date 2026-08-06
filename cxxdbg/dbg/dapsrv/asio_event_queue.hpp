// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file asio_event_queue.hpp
/// Contains definition of the asio_event_queue class.

#pragma once

#include "cxxdbg/async/async.hpp"
#include <boost/asio/executor.hpp>
#include <boost/asio/post.hpp>


namespace cxxdbg::dbg::dapsrv {


/// Event queue implementation for dbg core that posts events to ASIO executor
class asio_event_queue: virtual public async::event_queue {
public:
    /// Constructs event queue with specified ASIO executor
    asio_event_queue(boost::asio::executor exec):
        exec_{std::move(exec)} {}

    /// Posts event into queue
    void post(const std::function<void()> & func) override {
        boost::asio::post(exec_, func);
    }

private:
    boost::asio::executor exec_;        ///< ASIO executor
};


}
