// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file lldb_listener_event_queue.cpp
/// Contains implementation of the lldb_listener_event_queue class.

#include "lldb_listener_event_queue.hpp"
#include "cxxdbg/log/log.hpp"
#include <lldb/API/SBEvent.h>
#include <lldb/Utility/Event.h>


namespace cxxdbg::dbg::core {


/// Event type enum
enum class lldb_listener_event_queue_event: uint32_t {
    event = 1,
    quit = 2
};


/// LLDB Event data derived class for posting events to queue
class lldb_listener_event_queue_event_data: public lldb_private::EventData {
public:
    /// Constructs event data with specified event handler
    lldb_listener_event_queue_event_data(const std::function<void ()> & handl):
        handler_{handl} {}

    virtual llvm::StringRef GetFlavor() const override {
        return llvm::StringRef{"lldb_listener_event_queue_event_data"};
    }

    /// Executes event handler
    void exec_handler() const {
        handler_();
    }

private:
    std::function<void ()> handler_;
};


lldb_listener_event_queue::lldb_listener_event_queue(const lldb::SBListener & listener):
listener_{listener}, broadcaster_{"lldb_listener_event_queue"} {
    // add listener to broadcaster for listening all events
    broadcaster_.AddListener(listener_, 0xFFFFFFFF);
}


lldb_listener_event_queue::~lldb_listener_event_queue() = default;


void lldb_listener_event_queue::post(const std::function<void()> & func) {
    CXXDBG_LOG_SCAT_DEBUG(core, lldb_listener_event_queue) << "posting event to lldb queue";
    auto event_type = static_cast<uint32_t>(lldb_listener_event_queue_event::event);
    auto data = new lldb_listener_event_queue_event_data{func};
    auto event = std::make_shared<lldb_private::Event>(event_type, data);
    broadcaster_.BroadcastEvent(lldb::SBEvent{event});
}


void lldb_listener_event_queue::post_quit() {
    broadcaster_.BroadcastEventByType(static_cast<uint32_t>(lldb_listener_event_queue_event::quit));
}


void lldb_listener_event_queue::run() {
    while (true) {
        // extracting next lldb event
        lldb::SBEvent ev;
        listener_.WaitForEvent(UINT32_MAX, ev);

        if (ev.GetBroadcaster() == broadcaster_) {
            // event posted by this queue
            if (ev.GetType() == static_cast<uint32_t>(lldb_listener_event_queue_event::event)) {
                CXXDBG_LOG_SCAT_DEBUG(core, lldb_listener_event_queue) << "received post action event";
                auto event_data = static_cast<lldb_listener_event_queue_event_data*>(ev.GetSP()->GetData());
                assert(event_data != nullptr && "invalid event data for queue event");
                event_data->exec_handler();
            } else if (ev.GetType() == static_cast<uint32_t>(lldb_listener_event_queue_event::quit)) {
                // quit event
                break;
            } else {
                CXXDBG_LOG_SCAT_ERROR(core, lldb_listener_event_queue) << "unknown event type";
                assert(false && "unknown event type");
            }
        } else {
            // other lldb event, sending event signal
            CXXDBG_LOG_SCAT_DEBUG(core, lldb_listener_event_queue) << "received LLDB event";
            lldb_event_signal_(ev);
        }
    }
}


}
