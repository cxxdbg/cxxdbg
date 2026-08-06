// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file StepUntilDialog.cpp
/// Contains implementation of the StepUntilDialog class.

#include "StepUntilDialog.h"
#include "FormLayout.h"
#include "cxxdbg_connect.hpp"
#include <QDialogButtonBox>
#include <QIntValidator>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>
#include <cassert>
#include <limits>


namespace cxxdbg::gui {


StepUntilDialog::StepUntilDialog(QWidget * parent):
QDialog(parent) {
    setWindowTitle(tr("Step until"));
    QVBoxLayout * layout = new QVBoxLayout(this);

    // dialog form
    {
        auto formLayout = new FormLayout;
        layout->addLayout(formLayout);

        // PID row
        lineNumEdit_ = new QLineEdit;
        formLayout->addRow(tr("Line number") + ":", lineNumEdit_);
        lineNumEdit_->setValidator(new QIntValidator(1, std::numeric_limits<int>::max()));
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


unsigned int StepUntilDialog::lineNumber() const {
    bool is_ok = false;
    unsigned int lineNum = lineNumEdit_->text().toUInt(&is_ok);
    assert(is_ok && "line number is not unsigned integer");
    return lineNum;
}


void StepUntilDialog::onOkClicked() {

    // checking that line number is not empty
    if (lineNumEdit_->text().isEmpty()) {
        QMessageBox msg(QMessageBox::Critical, tr("Error"),
                        tr("Line number is not specified. "
                           "Please enter correct line number."));
        msg.exec();
        return;
    }

    accept();
}


}
