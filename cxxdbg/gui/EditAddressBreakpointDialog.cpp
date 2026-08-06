// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file EditAddressBreakpointDialog.cpp
/// Contains implementation of EditAddressBreakpointDialog class.

#include "EditAddressBreakpointDialog.h"
#include "FormLayout.h"
#include "HexValidator.h"
#include "cxxdbg_connect.hpp"
#include <sstream>
#include <QApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>


namespace cxxdbg::gui {


EditAddressBreakpointDialog::EditAddressBreakpointDialog(QWidget * parent):
QDialog(parent) {
    setWindowTitle(tr("Edit breakpoint address"));
    QVBoxLayout * layout = new QVBoxLayout(this);

    // description label
    layout->addWidget(new QLabel(tr(
        "Program execution will be stopped after reaching\n"
        "specified address.")));


    // address
    auto formLayout = new FormLayout;
    layout->addLayout(formLayout);
    addrEdit_ = new QLineEdit;
    formLayout->addRow(tr("Breakpoint address:"), addrEdit_);
    addrEdit_->setValidator(new HexValidator);

    // TODO: set maximum text width depending on address size
    addrEdit_->setMaxLength(16);

    // dialog buttons
    QDialogButtonBox * buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    cxxdbg_connect(buttons, &QDialogButtonBox::rejected, [this] { reject(); });

    cxxdbg_connect(buttons, &QDialogButtonBox::accepted, [this]() {
        // checking that address is not empty
        if (addrEdit_->text().isEmpty()) {
            QMessageBox msg(QMessageBox::Critical,
                            tr("Error"),
                            tr("Breakpoint address is not specified. Please specify breakpoint address."),
                            QMessageBox::Ok);
            msg.exec();
            addrEdit_->setFocus();
            return;
        }

        accept();
    });
}


std::uint64_t EditAddressBreakpointDialog::address() const {
    std::istringstream str(addrEdit_->text().toStdString());
    std::uint64_t addr;
    str >> std::hex >> addr;
    return addr;
}


}
