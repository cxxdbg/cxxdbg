// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file TerminalSettingsWidget.h
/// Contains definition of the TerminalSettingsWidget class.

#pragma once

#include "cxxdbg/dbg/term_settings.hpp"
#include <QWidget>


class QCheckBox;
class QGroupBox;
class QLineEdit;
class QRadioButton;


namespace cxxdbg::gui {


/// \class TerminalSettingsWidget
/// Displays terminal settings
class TerminalSettingsWidget: public QWidget {
    Q_OBJECT

public:
    TerminalSettingsWidget(const cxxdbg::dbg::term_settings & sett,
                           QWidget * parent = nullptr);

    /// Returns terminal settings entered by user
    cxxdbg::dbg::term_settings termSettings() const;

signals:
    void changed();

private:
    /// Updates group states
    void updateGroups();

    QRadioButton * builtinButton_;      ///< "Use builtin emulator" button
    QRadioButton * xtermButton_;        ///< "Use xterm" button
    QRadioButton * customButton_;       ///< "Use custom terminal" button
    QGroupBox * xtermGroup_;            ///< xterm settings group
    QLineEdit * xtermPathEdit_;         ///< Line edit for path to xterm
    QCheckBox * xtermCloseCheck_;       ///< "Close xterm after exit" check box
    QGroupBox * customGroup_;           ///< Custom terminal settings group
    QLineEdit * customCommandEdit_;     ///< Command line for custom terminal
};


}
