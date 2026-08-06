// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file AttachDialog.h
/// Contains definition of the AttachDialog class.

#pragma once

#include <QDialog>


class QLineEdit;

namespace cxxdbg::dbg {
    class application;
}


namespace cxxdbg::gui {

class PlatformWidget;


/// \class AttachDialog
/// Dialog for entering options for attaching to process
class AttachDialog: public QDialog {
    Q_OBJECT

public:
    /// Constructors attach dialog with specified parent
    AttachDialog(cxxdbg::dbg::application & app, QWidget * parent = 0);

    /// Returns PID
    unsigned long pid() const;

private:
    /// Called when user clicks Ok buttom
    void onOkClicked();

    cxxdbg::dbg::application & app_;  ///< Reference to application object
    QLineEdit * pidLineEdit_;       ///< PID line edit
    PlatformWidget * platformOpts_; ///< Platform options
};


}
