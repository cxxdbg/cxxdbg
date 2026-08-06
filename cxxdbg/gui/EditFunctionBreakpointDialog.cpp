// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file EditFunctionBreakpointDialog.cpp
/// Contains implementation of EditFunctionBreakpointDialog class.

#include "EditFunctionBreakpointDialog.h"
#include "FormLayout.h"
#include "cxxdbg_connect.hpp"
#include "cxxdbg/dbg/debugger.hpp"
#include "cxxdbg/dbg/target.hpp"
#include <QApplication>
#include <QCompleter>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>


namespace cxxdbg::gui {


EditFunctionBreakpointDialog::EditFunctionBreakpointDialog(cxxdbg::dbg::debugger & dbg,
                                                           QWidget * parent):
QDialog{parent},
dbg_{dbg} {
    setWindowTitle(tr("Edit breakpoint position"));
    QVBoxLayout * layout = new QVBoxLayout(this);

    // description label
    layout->addWidget(new QLabel(tr(
        "Program execution will be stopped after reaching\n"
        "specified function.")));


    // function name
    auto formLayout = new FormLayout;
    layout->addLayout(formLayout);
    funcNameEdit_ = new QLineEdit;
    QCompleter * c = new QCompleter;
    c->setModel(&qtFuncsModel_);
    funcNameEdit_->setCompleter(c);
    formLayout->addRow(tr("Function name:"), funcNameEdit_);
    QFontMetrics metrics(QApplication::font());
    funcNameEdit_->setMinimumWidth(metrics.horizontalAdvance(QString(40, '0')));

    cxxdbg_connect(funcNameEdit_, &QLineEdit::textEdited, [this](const QString & text) {
        if (dbg_.has_target()) {
            funcsModel_.set_strings(nullptr);
            funcs_ = dbg_.current_target().find_funcs(text.toStdString());
            funcsModel_.set_strings(&funcs_);
        }
    });

    // dialog buttons
    QDialogButtonBox * buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    cxxdbg_connect(buttons, &QDialogButtonBox::rejected, [this] { reject(); });

    cxxdbg_connect(buttons, &QDialogButtonBox::accepted, [this]() {
        // checking that function name is not empty
        if (funcName().isEmpty()) {
            QMessageBox msg(QMessageBox::Critical,
                            tr("Error"),
                            tr("Function name is not specified. Please specify function name."),
                            QMessageBox::Ok);
            msg.exec();
            funcNameEdit_->setFocus();
            return;
        }

        accept();
    });
}


QString EditFunctionBreakpointDialog::funcName() const {
    return funcNameEdit_->text();
}


}
