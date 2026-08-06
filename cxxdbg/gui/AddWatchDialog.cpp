// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file AddWatchDialog.cpp
/// Contains implementation of the AddWatchDialog class.

#include "AddWatchDialog.h"
#include "FormLayout.h"
#include "cxxdbg_connect.hpp"
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>


namespace cxxdbg::gui {


AddWatchDialog::AddWatchDialog(QWidget * parent):
QDialog(parent) {
    setWindowTitle(tr("Add watch"));
    QVBoxLayout * layout = new QVBoxLayout(this);

    // dialog form
    {
        auto formLayout = new FormLayout;
        layout->addLayout(formLayout);

        // watch expression
        watchExprEdit_ = new QLineEdit;
        formLayout->addRow(tr("Watch expression") + ":", watchExprEdit_);
    }

    // dialog buttons
    {
        QDialogButtonBox * buttons =
                new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        layout->addWidget(buttons);

        cxxdbg_connect(buttons, &QDialogButtonBox::accepted, [this] { onOkClicked(); });
        cxxdbg_connect(buttons, &QDialogButtonBox::rejected, [this] { reject(); });
    }


    setFixedHeight(sizeHint().height());
}


QString AddWatchDialog::watchExpr() const {
    return watchExprEdit_->text();
}


void AddWatchDialog::onOkClicked() {
    // checking that watch expression is not empty
    if (watchExprEdit_->text().isEmpty()) {
        QMessageBox msgBox{QMessageBox::Critical,
                           tr("Error"),
                           tr("Watch expression should not be empty. Please enter "
                              "correct watch expression"),
                           QMessageBox::Ok};
        msgBox.exec();
        return;
    }

    accept();
}


}
