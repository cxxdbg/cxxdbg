// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file StepUntilDialog.h
/// Contains definition of the StepUntiDialog class.

#pragma once

#include <QDialog>


class QLineEdit;


namespace cxxdbg::gui {


/// \class StepUntilDialog
/// Dialog for entering parameters for step until action
class StepUntilDialog: public QDialog {
public:
    /// Constructor, makes step until dialog
    StepUntilDialog(QWidget * parent);

    /// Returns line number
    unsigned int lineNumber() const;

private:
    /// Called when user clicks ok button
    void onOkClicked();

    QLineEdit * lineNumEdit_;           ///< Line number widget
};


}
