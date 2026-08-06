// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "cxxdbg/dbg/launch_options.hpp"
#include "cxxdbg/dbg/platform.hpp"
#include <QDialog>


class QLabel;
class QLineEdit;
class QStackedWidget;

namespace cxxdbg::dbg {
    class application;
}


namespace cxxdbg::gui {

class FileSelectWidget;
class LaunchOptionsWidget;
class PlatformWidget;
class CxxdbgApplication;


/// Dialog for selecting executable and load options
class LoadExecutableDialog : public QDialog {
    Q_OBJECT
public:
    /// Constructor, makes load dialog
    explicit LoadExecutableDialog(cxxdbg::dbg::application & app, QWidget *parent = 0);

    /// Returns name of selected executable file
    QString fileName() const;

    /// Returns default launch options
    cxxdbg::dbg::launch_options defLaunchOpts() const;

    /// Returns platform connection options
    cxxdbg::dbg::platform_connection_options platformOpts() const;

private:
    /// Called when user clicks Ok button
    void onOkClicked();

    /// Called when selected platform changes
    void onSelectedPlatformChanged();

    cxxdbg::dbg::application & app_;      ///< Reference to application object
    QLabel * targetLabel_;              ///< Label for target field
    FileSelectWidget * fileName_;       ///< Exe file select widget
    LaunchOptionsWidget * launchOpts_;  ///< Launch options
    PlatformWidget * platformOpts_;     ///< Platform options
};


}
