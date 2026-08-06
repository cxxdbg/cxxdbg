// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file PlatformWidget.h
/// Cotnains definition of the PlatformWidget class.

#pragma once

#include "cxxdbg/dbg/platform.hpp"
#include <QWidget>


class QComboBox;
class QLineEdit;
class QPlainTextEdit;

namespace cxxdbg::dbg {
    class platform_list;
}


namespace cxxdbg::gui {


/// Platform for selecting platform connection settings
class PlatformWidget: public QWidget {
    Q_OBJECT;

public:
    /// Constructs widget with specified list of platforms and parent
    PlatformWidget(const cxxdbg::dbg::platform_list & plist,
                   bool displayHost,
                   bool diplayCurrent,
                   QWidget * parent = nullptr);

    /// Returns selected platform or nullptr if no platform is selected
    const cxxdbg::dbg::platform * selectedPlatform() const;

    /// Returns URL
    QString url() const;

    /// Returns executable search paths
    std::vector<std::string> execSearchPaths() const;

    /// Returns platform connection options
    cxxdbg::dbg::platform_connection_options platformOpts() const;

    /// Checks that all required fields are not empty. Displays error message if not.
    /// Returns true if all fiedls are correct
    bool checkOptions();

signals:
    /// The signal is emitted when selected platform changes
    void selectedPlatformChanged();

private:
    /// Called when selected item in platform combol is changed
    void onSelectedPlatformChanged();

    const cxxdbg::dbg::platform_list & platforms_;    ///< List of platforms
    bool displayCurrent_ = false;                   ///< Display current platform in list of platforms
    QComboBox * platformCombo_ = nullptr;           ///< Platform selection combo box
    QLineEdit * urlLineEdit_ = nullptr;             ///< Line edit containing platform URL
    QPlainTextEdit * searchPaths_ = nullptr;        ///< Executable search path
};


}
