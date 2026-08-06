// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "FunctionNameDialog.h"
#include "cxxdbg_connect.hpp"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

#include <assert.h>

#include "ui_FunctionNameDialog.h"


namespace cxxdbg::gui {


FunctionNameDialog::FunctionNameDialog(QWidget * parent) {
    Ui::FunctionNameDialog ui;

    ui.setupUi(this);

    functionNameEdit_ = ui.functionNameEdit;
    QDialogButtonBox * btnBox = ui.buttonBox;
    okButton_ = btnBox->button(QDialogButtonBox::Ok);
    statusLabel_ = ui.statusLabel;

    updateState();

    cxxdbg_connect(functionNameEdit_, &QLineEdit::textChanged, [this](const QString & text) {
        functionName_ = text;
        updateState();
    });
}


QString FunctionNameDialog::getFunctionName() {
    return functionName_;
}


void FunctionNameDialog::updateState() {
    okButton_->setEnabled(!functionName_.isEmpty());
    bool res = false;
    QString status;
    std::tie(res, status) = validate();

    if (res) {
        statusLabel_->setText("");
    } else {
        statusLabel_->setText(status);
    }
}


std::pair<bool, QString> FunctionNameDialog::validate() {
    if (functionName_.isEmpty()) {
        return {false, tr("Function name should not be empty")};
    }

    return {true, {}};
}


}
