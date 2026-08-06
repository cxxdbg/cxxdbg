// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file EditSourcePosBreakpointDialog.cpp
/// Contains implementation of EdutSourcePosBreakpointDialog class.

#include "EditSourcePosBreakpointDialog.h"
#include "FormLayout.h"
#include "cxxdbg_connect.hpp"

#include <cassert>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>


namespace cxxdbg::gui {


EditSourcePosBreakpointDialog::EditSourcePosBreakpointDialog(const cxxdbg::ro_tree_view_model * srcList,
                                                             QWidget * parent):
QDialog(parent) {

    setWindowTitle(tr("Edit breakpoint position"));
    QVBoxLayout * layout = new QVBoxLayout(this);

    // description label
    layout->addWidget(new QLabel(tr(
        "Program execution will be stopped after reaching\n"
        "specified position in source code.")));

    // dialog form
    {
        auto formLayout = new FormLayout;
        layout->addLayout(formLayout);

        // file selection widget
        sourceNameWidget_ = new QComboBox;
        formLayout->addRow(tr("File:"), sourceNameWidget_);
        sourceNameWidget_->setEditable(true);
        sourceNameWidget_->setInsertPolicy(QComboBox::NoInsert);
        sourceNameWidget_->setCurrentText({});

        // creating qt model for source file name if source list is not null
        if (srcList != nullptr) {
            qtSrcListModel_ = std::make_unique<AppRoItemModel>(*srcList);
            sourceNameWidget_->setModel(qtSrcListModel_.get());
            sourceNameWidget_->setCurrentText({});
        }

        // line number widget
        lineNumEdit_ = new QLineEdit;
        QIntValidator * val = new QIntValidator;
        lineNumEdit_->setValidator(val);
        val->setBottom(1);
        formLayout->addRow(tr("Line:"), lineNumEdit_);
    }

    // dialog buttons
    QDialogButtonBox * buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    cxxdbg_connect(buttons, &QDialogButtonBox::rejected, [this] { reject(); });

    cxxdbg_connect(buttons, &QDialogButtonBox::accepted, [this]() {
        // checking that file name is not empty
        QString fName = fileName();
        if (fName.isEmpty()) {
            QMessageBox msg(QMessageBox::Critical,
                            tr("Error"),
                            tr("File name is not specified. Please specify file name."),
                            QMessageBox::Ok);
            msg.exec();
            return;
        }

        // checking that line number is not empty
        QString lineNum = lineNumEdit_->text();
        if (lineNum.isEmpty()) {
            QMessageBox msg(QMessageBox::Critical,
                            tr("Error"),
                            tr("Line number is not specified. Please specify line number."),
                            QMessageBox::Ok);
            msg.exec();
            return;
        }

        // checking line number
        bool is_ok = false;
        unsigned int lNum = lineNum.toUInt(&is_ok);
        if (!is_ok || lNum == 0) {
            QMessageBox msg(QMessageBox::Critical,
                            tr("Error"),
                            tr("Line number is not correct. Please specify correct line number."),
                            QMessageBox::Ok);
            msg.exec();
            return;
        }

        accept();
    });
}


QString EditSourcePosBreakpointDialog::fileName() const {
    return sourceNameWidget_->currentText();
}


void EditSourcePosBreakpointDialog::setFileName(const QString & nm) {
    sourceNameWidget_->setCurrentText(nm);
}


unsigned int EditSourcePosBreakpointDialog::lineNumber() const {
    bool is_ok = false;
    unsigned lineNum = lineNumEdit_->text().toUInt(&is_ok);
    assert(is_ok && "line number is not unsigned integer");
    return lineNum;
}


void EditSourcePosBreakpointDialog::setLineNumber(unsigned int line) {
    lineNumEdit_->setText(QString::number(line));
}


}
