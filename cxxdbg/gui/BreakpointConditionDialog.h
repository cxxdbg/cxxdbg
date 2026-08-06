// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file BreakpointConditionDialog.h
/// Contains definition of BreakpointConditionDialog class.

#pragma once

#include <QDialog>


class QCheckBox;
class QLineEdit;


namespace cxxdbg::gui {

class FormLayout;


/// \class BreakpointConditionDialog
/// Dialog for editing breakpoint condition
class BreakpointConditionDialog: public QDialog {
    Q_OBJECT

public:
    /// Constructor, makes dialog with specified parent
    BreakpointConditionDialog(QWidget * parent);

    /// Returns condition
    QString condition() const;

    /// Sets condition
    void setCondition(const QString & cond);

private:
    /// Called when user accepts dialog
    void onAccepted();

    /// Called when enable checkbox state changed
    void onEnableStateChanged();

    FormLayout * formLayout_;       ///< Form layout
    QCheckBox * enableCheckBox_;    ///< Enable condition check box
    QLineEdit * conditionLineEdit_; ///< Condition line edit
};


}