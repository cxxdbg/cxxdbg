// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file TerminalDockWidget.hpp
/// Contains definition of the TerminalDockWidget class

#pragma once

#include "CxxdbgDockWidget.h"
#include "cxxdbg/app/signals.hpp"


namespace cxxdbg {
    class terminal;
}


namespace cxxdbg::gui {

class TerminalWidget;


/// Dock widget containing executable terminal emulator
class TerminalDockWidget: public CxxdbgDockWidget {
public:
    /// Constructs terminal widget with specified reference to application
    /// executable terminal
    TerminalDockWidget(cxxdbg::terminal &term);
};


}
