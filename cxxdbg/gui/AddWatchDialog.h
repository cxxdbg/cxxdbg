// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file AddWatchDialog.h
/// Contains definition of the AddWatchDialog class.

#pragma once

#include <QDialog>


class QLineEdit;


namespace cxxdbg::gui {


/// Dialog for adding watch into watch list
class AddWatchDialog: public QDialog {
public:
    /// Constructor, makes dialog
    AddWatchDialog(QWidget * parent);

    /// Returns watch expression
    QString watchExpr() const;

private:
    /// Called when user clicks ok button
    void onOkClicked();

    QLineEdit * watchExprEdit_;     ///< Watch expression line edit
};


}
