// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file ThreadsDockWidget.h
/// Contains definition of ThreadsDockWidget class.

#pragma once

#include "CxxdbgDockWidget.h"
#include "cxxdbg/app/signals.hpp"
#include <QIcon>


class QAction;

namespace cxxdbg {
    class document_navigator;
}

namespace cxxdbg::dbg {
    class debugger;
}


namespace cxxdbg::gui {

class DebugUI;
class CxxdbgTreeWidget;


/// \class ThreadsDockWidget
/// Dock widget which displays list of threads in process
/// being debugged.
class ThreadsDockWidget: public CxxdbgDockWidget {
public:
    /// Constructor, makes threads dock widget with specified reference
    /// to debugger object, list of documents, common debug UI implementation, and parent
    ThreadsDockWidget(cxxdbg::dbg::debugger & dbg,
                      document_navigator & doc_nav,
                      DebugUI & dbgUi,
                      QWidget * parent = nullptr);

    /// Saves state of dock widget
    void saveState();

private:
    /// Converts thread id to string
    QString threadIdToString(unsigned long id);

    /// Called when application state changed
    void onStateChanged();

    /// Called when current thread is changed
    void onCurrentThreadChanged();

    /// Called when user selects copy action
    void onCopy();

    /// Called when user toggles hexadecimal display option
    void onHexDisplay(bool isHex);

    cxxdbg::dbg::debugger & dbg_;         ///< Reference to debugger object
    document_navigator & doc_nav_;      ///< Reference to document navigator
    DebugUI & dbgUi_;                   ///< Reference to common debug UI

    /// Connection to state changed signal
    cxxdbg::scoped_signal_connection stateChangedCon_;

    /// Connection to current thread changed signal
    cxxdbg::scoped_signal_connection currentThreadChangedCon_;

    CxxdbgTreeWidget * threads_;          ///< Thread list widget
    QAction * copyAction_;              ///< Copy action
    QAction * hexDisplayAction_;        ///< Hexadecimal display action
    QAction * selectThreadAction_;      ///< Select thread action
    QAction * goToSourceAction_;        ///< Go to source action
    QIcon currentIcon_;                 ///< Current thread icon
};


}
