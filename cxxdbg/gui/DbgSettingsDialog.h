// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file DbgSettingsDialog.h
/// Contains definition of the DbgSettingsDialog class.

#pragma once

#include "SettingsDialog.h"


class QCheckBox;
class QDialogButtonBox;
class QLineEdit;
class QRadioButton;


namespace cxxdbg::gui {


class FontSelectWidget;
class CxxdbgApplication;
class TerminalSettingsWidget;
class DebugSettingsWidget;


/// \class DbgSettingsDialog
/// Settings dialog
class DbgSettingsDialog: public SettingsDialog {
    Q_OBJECT

public:
    DbgSettingsDialog(CxxdbgApplication & app, QWidget * parent = nullptr);

signals:
    /// Emitted when user clicks apply button
    void applyClicked();

private:
    /// Called when user clicks apply button
    void onApplyClicked();

    /// Called when user changed settings
    void onChanged();

    CxxdbgApplication &        app_;              ///< Reference to application object
    FontSelectWidget *       fontSelect_;       ///< Font select widget
    TerminalSettingsWidget * termSettings_;     ///< Terminal settings widget
    DebugSettingsWidget *    debugSettings_;    ///< Debug settings widget
};


}
