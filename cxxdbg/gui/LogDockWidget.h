// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file LogDockWidget.h
/// Contains definition of the LogDockWidget class.

#pragma once

#include "CxxdbgDockWidget.h"
#include "cxxdbg/app/application_log.hpp"
#include <QPlainTextEdit>


namespace cxxdbg::gui {


/// Dock widget for displaying log
class LogDockWidget: public CxxdbgDockWidget {
public:
    /// Constructs log dock widget with specified referece to application log
    LogDockWidget(cxxdbg::application_log & log);

    void showEvent(QShowEvent * event) override;
    void hideEvent(QHideEvent * event) override;

private:
    cxxdbg::application_log & log_;               ///< Reference to application log object
    cxxdbg::util::scoped_signal_connection con_;  ///< Log connection

    QPlainTextEdit * logEdit_;                  ///< Text edit for displaying log
};


}
