// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file lldb_listener_event_queue
/// Contains definition of the lldb_listener_event_queue class.

#pragma once

#include "cxxdbg/async/event_queue.hpp"
#include <lldb/API/SBBroadcaster.h>
#include <lldb/API/SBListener.h>
#include <boost/signals2/signal.hpp>


namespace cxxdbg::dbg::core {


/// Event queue implemented on top of lldb listener
class lldb_listener_event_queue: public async::event_queue {
public:
    /// Constructs event queue using specified LLDB listener object
    lldb_listener_event_queue(const lldb::SBListener & listener);

    /// Default destructor
    ~lldb_listener_event_queue() override;

    /// Posts event into queue
    void post(const std::function<void()> & func) override;

    /// Posts quit event
    void post_quit();

    /// Processes queue events until quit event is received
    void run();

    /// Returns reference to lldb event signal
    auto & lldb_event_signal() { return lldb_event_signal_; }

private:
    lldb::SBListener listener_;             ///< LLDB listener
    lldb::SBBroadcaster broadcaster_;       ///< lldb broadcaster for posting events

    /// LLDB event signal
    boost::signals2::signal<void (const lldb::SBEvent &)> lldb_event_signal_;
};


}
