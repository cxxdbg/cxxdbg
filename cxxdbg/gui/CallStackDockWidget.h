// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file CallStackDockWdiget
/// Contains definition of CallStackDockWidget class.

#pragma once

#include <QIcon>

#include "AppItemModel.h"
#include "CxxdbgDockWidget.h"

class QAction;

namespace cxxdbg {
    class document_navigator;
}

namespace cxxdbg::dbg {
    class call_stack_model;
    class debugger;
    class source_position;
}


namespace cxxdbg::gui {

class DebugUI;
class CxxdbgTreeView;


/// \class CallStackDockWdiget
/// Dock widget which displays call stack for active thread.
class CallStackDockWidget: public CxxdbgDockWidget {
    Q_OBJECT
public:
    CallStackDockWidget(cxxdbg::dbg::debugger & dbg,
                        cxxdbg::document_navigator & doc_nav,
                        DebugUI & dbgUi,
                        QWidget * parent = nullptr);

    /// Saves state of call stack dock panel
    void saveState();

private:
    /// Updates actions states depending on selection
    void updateActions();

    cxxdbg::dbg::debugger & dbg_;             ///< Reference to debugger object
    cxxdbg::document_navigator & doc_nav_;    ///< Reference to document navigator
    cxxdbg::dbg::call_stack_model & model_;   ///< Reference to call stack model
    AppRoItemModel qtModel_;                ///< Qt call stack model
    CxxdbgTreeView * callStack_;              ///< Call stack list
    QIcon topFrameIcon_;                    ///< Icon for top frame

    QAction * switchToFrameAction_; ///< Switch to frame action
    QAction * goToSourceAction_;    ///< Go to source action
    QAction * hexDisplayAction_;    ///< Hexadecimal display context menu action
    QAction * displayTypesAction_;  ///< Display parameter types context menu action
    QAction * displayNamesAction_;  ///< Display parameter names context menu action
    QAction * displayValuesAction_; ///< Display parameter values context menu action
};


}
