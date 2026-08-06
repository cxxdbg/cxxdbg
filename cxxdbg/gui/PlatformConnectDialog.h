// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file PlatformConnectDialog.h
/// Contains definition of the PlatformConnectDialog class.

#pragma once

#include <QDialog>


namespace cxxdbg::dbg {
    class application;
}


namespace cxxdbg::gui {

class PlatformWidget;
class ProgressWidget;


/// Dialog for connecting to platform
class PlatformConnectDialog: public QDialog {
public:
    /// Constructs dialog with specified reference to application and parent widget
    PlatformConnectDialog(cxxdbg::dbg::application & app, QWidget * parent = nullptr);

private:
    /// Called when ok button pressed
    void onOkClicked();

    cxxdbg::dbg::application & app_;  ///< Reference to application
    PlatformWidget * platform_;     ///< Platform settings widget
    ProgressWidget * progress_;     ///< Widget for displaying connecting progress
};


}
