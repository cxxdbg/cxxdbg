// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file BreakpointConditionDialog.cpp
/// Contains implementation of BreakpointConditionDialog class.

#include "BreakpointConditionDialog.h"
#include "FormLayout.h"
#include "cxxdbg_connect.hpp"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPalette>
#include <QVBoxLayout>


namespace cxxdbg::gui {


BreakpointConditionDialog::BreakpointConditionDialog(QWidget * parent):
QDialog(parent) {
    setWindowTitle(tr("Breakpoint condition"));

    QVBoxLayout * layout = new QVBoxLayout(this);

    formLayout_ = new FormLayout;
    layout->addLayout(formLayout_);

    formLayout_->addRow(new QLabel(tr(
        "Breakpoint with condition evaluates the expression each\n"
        "time process reaches breakpoint location. Execution stops\n"
        "only if the condition is true.")));

    // enable check box
    enableCheckBox_ = new QCheckBox(tr("Enable breakpoint condition"));
    formLayout_->addRow(enableCheckBox_);
    cxxdbg_connect(enableCheckBox_, &QCheckBox::stateChanged, [this](auto && ...) { onEnableStateChanged(); });

    // condition line edit
    conditionLineEdit_ = new QLineEdit;
    formLayout_->addRow(tr("Condition:"), conditionLineEdit_);

    // dialog buttons
    QDialogButtonBox * buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);

    cxxdbg_connect(buttons, &QDialogButtonBox::accepted, [this] { onAccepted(); });
    cxxdbg_connect(buttons, &QDialogButtonBox::rejected, [this] { reject(); });

    onEnableStateChanged();
}


QString BreakpointConditionDialog::condition() const {
    return enableCheckBox_->isChecked() ? conditionLineEdit_->text() : QString();
}


void BreakpointConditionDialog::setCondition(const QString & cond) {
    conditionLineEdit_->setText(cond);
    enableCheckBox_->setChecked(!cond.isEmpty());
}


void BreakpointConditionDialog::onAccepted() {

    // checking that condition is not empty if enabled
    if (enableCheckBox_->isChecked()) {
        if (conditionLineEdit_->text().isEmpty()) {
            QMessageBox msg(QMessageBox::Critical,
                            tr("Error"),
                            tr("Condition expression is empty. Please insert "
                               "correct condition expression."));
            msg.exec();
            return;
        }
    }

    accept();
}


void BreakpointConditionDialog::onEnableStateChanged() {
    conditionLineEdit_->setEnabled(enableCheckBox_->isChecked());
    formLayout_->labelForField(conditionLineEdit_)->setEnabled(enableCheckBox_->isChecked());
}


}
