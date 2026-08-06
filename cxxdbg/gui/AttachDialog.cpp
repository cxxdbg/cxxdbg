// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file AttachDialog.cpp
/// Contains implementation of the AttachDialog class.

#include "AttachDialog.h"
#include "FormLayout.h"
#include "PlatformWidget.h"
#include "cxxdbg_connect.hpp"

#include "cxxdbg/dbg/application.hpp"
#include "cxxdbg/dbg/debugger.hpp"

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QIntValidator>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>
#include <cassert>
#include <limits>


namespace cxxdbg::gui {


AttachDialog::AttachDialog(cxxdbg::dbg::application & app, QWidget * parent):
QDialog(parent),
app_{app} {
    setWindowTitle(tr("Attach to process"));
    QVBoxLayout * layout = new QVBoxLayout(this);

    // dialog form
    {
        auto formLayout = new FormLayout;
        layout->addLayout(formLayout);

        // PID row
        pidLineEdit_ = new QLineEdit;
        formLayout->addRow(tr("Process ID or name") + ":", pidLineEdit_);

        // platform options
        QGroupBox * platformGrp = new QGroupBox{tr("Platform options")};
        layout->addWidget(platformGrp);
        QVBoxLayout * platformGrpLayout = new QVBoxLayout{platformGrp};
        platformOpts_ = new PlatformWidget{app_.dbg().platforms(), true, true};
        platformGrpLayout->addWidget(platformOpts_);
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


unsigned long AttachDialog::pid() const {
    bool isOk = false;
    unsigned long p = pidLineEdit_->text().toULong(&isOk);
    return p;
}


void AttachDialog::onOkClicked() {
    // checking that PID is not empty
    if (pidLineEdit_->text().isEmpty()) {
        QMessageBox msg(QMessageBox::Critical, tr("Error"),
                        tr("Process ID is not specified. "
                           "Please enter correct process ID."));
        msg.exec();
        return;
    }

    // checking platform options
    if (!platformOpts_->checkOptions()) {
        return;
    }

    // trying parse process ID from text field
    std::istringstream str{pidLineEdit_->text().toStdString()};
    unsigned long pid = 0;
    std::variant<unsigned long, std::string> targ_name;
    if ((str >> pid) && str.eof() && pid != 0) {
        targ_name = pid;
    } else {
        targ_name = pidLineEdit_->text().toStdString();
    }

    // attaching
    if (!app_.ask_and_attach(targ_name, platformOpts_->platformOpts())) {
        return;
    }

    // closing dialog if attach is done
    accept();
}


}
