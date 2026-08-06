// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file EventQueueApplication.cpp
/// Contains implementation of the EventQueueApplication.

#include "EventQueueApplication.h"


namespace cxxdbg::gui {


namespace {


/// Helper event for cxxdbg::async::event_queue implementation
class AsyncEvent: public QEvent {
public:
    /// Constructor, makes event with specified handler
    AsyncEvent(const std::function<void()> & handl):
        QEvent(QEvent::User), handler_(handl) {}

    /// Executes event handler
    void executeHandler() const {
        handler_();
    }

private:
    std::function<void()> handler_;
};


}


void EventQueueApplication::post(const std::function<void ()> & func) {
    postEvent(this, new AsyncEvent(func));
}


bool EventQueueApplication::event(QEvent * evnt) {
    AsyncEvent * aevent = dynamic_cast<AsyncEvent*>(evnt);

    // process other events
    if (aevent == nullptr) {
        return QApplication::event(evnt);
    }

    // execute handler
    aevent->executeHandler();
    return true;
}


}
