// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file EventQueueApplication.h
/// Contains definition of the EventQueueApplication class.

#pragma once

#include "cxxdbg/async/event_queue.hpp"
#include <QApplication>


namespace cxxdbg::gui {


/// Qt application class that implements cxxdbg::async::event queue interface
/// and dispatches posted events in main Qt thread
class EventQueueApplication: public QApplication,
                             virtual public cxxdbg::async::event_queue {
public:
    /// Constructs application object and redirects passed arguments to
    /// QApplication constructor
    EventQueueApplication(int & argc, char ** argv):
        QApplication{argc, argv} {}

    /// Puts event into queue
    void post(const std::function<void()> & func) override;

protected:
    /// Processes Qt event and dispatches all messages posted with post method
    bool event(QEvent * evnt) override;
};


}
