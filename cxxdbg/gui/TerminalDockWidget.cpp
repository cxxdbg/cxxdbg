// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file TerminalDockWidget.cpp
/// Contains implementation of the TerminalDockWidget class.

#include "TerminalDockWidget.h"
#include "TerminalWidget.h"
#include "cxxdbg/app/terminal.hpp"
#include <QVBoxLayout>


namespace cxxdbg::gui {


TerminalDockWidget::TerminalDockWidget(cxxdbg::terminal & term):
CxxdbgDockWidget{tr("Executable terminal")} {
    setObjectName("exe_terminal_dock");
    auto termWidget = new TerminalWidget{term};
    setWidget(termWidget);
    termWidget->setFrameStyle(QFrame::NoFrame);
}


}
