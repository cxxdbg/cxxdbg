// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "AddWatchpointDialog.hpp"
#include "cxxdbg/dbg/breakpoint_list.hpp"
#include "cxxdbg_connect.hpp"

#include <assert.h>

#include <QRadioButton>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QIntValidator>
#include <QMetaObject>


#include "ui_AddWatchpointDialog.h"

namespace Ui {
    class AddWatchpointDlg;
}


namespace cxxdbg::gui {


AddWatchpointDialog::AddWatchpointDialog(cxxdbg::dbg::breakpoint_list & bplist, QWidget * parent):
        QDialog(parent) {

    Ui::AddWatchpointDlg().setupUi(this);

    variableRadio_ = findChild<QRadioButton *>("variableRadioButton");
    variableEdit_ = findChild<QLineEdit *>("variableLineEdit");
    variableLabel_ = findChild<QLabel *>("variableLabel");

    expressionRadio_ = findChild<QRadioButton *>("expressionRadioButton");
    expressionEdit_ = findChild<QLineEdit *>("expressionLineEdit");
    expressionLabel_ = findChild<QLabel *>("expressionLabel");
    readCheckBox_ = findChild<QCheckBox *>("stopWhenReadCheckBox");
    changedCheckBox_ = findChild<QCheckBox *>("stopWhenChangedCheckBox");
    sizeEdit_ = findChild<QLineEdit *>("sizeEdit");

    statusLabel_ = findChild<QLabel *>("statusLabel");
    buttons_ = findChild<QDialogButtonBox *>();
    okButton_ = buttons_->button(QDialogButtonBox::Ok);

    assert(variableEdit_ != nullptr && "variable radio button not found");
    assert(variableEdit_ != nullptr && "variable edit not found");
    assert(expressionRadio_ != nullptr && "expression radio button not found");
    assert(expressionEdit_ != nullptr && "expression edit not found");
    assert(readCheckBox_ != nullptr && "location specified by variable check box not found");
    assert(changedCheckBox_ != nullptr && "location specified by address expression not found");
    assert(statusLabel_ != nullptr && "status label not found");
    assert(buttons_ != nullptr && "dialog buttons not found");
    assert(okButton_ != nullptr && "the OK button not found");
    assert(sizeEdit_ != nullptr && "size line edit not found");

    sizeEdit_->setValidator(new QIntValidator(0, 1000000, this)); // accept only integers from 0 to 1 mln

    initDialogState();
    updateState();

    cxxdbg_connect(variableRadio_, &QRadioButton::clicked, [this](auto && ...) {
        data_.is_variable = variableRadio_->isChecked();
        data_.is_expression = !variableRadio_->isChecked();

        this->updateState();
    });

    cxxdbg_connect(expressionRadio_, &QRadioButton::clicked, [this](auto && ...) {
        data_.is_expression = expressionRadio_->isChecked();
        data_.is_variable = !expressionRadio_->isChecked();

        this->updateState();
    });

    cxxdbg_connect(variableEdit_, &QLineEdit::textChanged, [this](const QString & text) {
        data_.variable = text;
        this->updateState();
    });

    cxxdbg_connect(expressionEdit_, &QLineEdit::textChanged, [this](const QString & text) {
        data_.expression = text;
        this->updateState();
    });

    cxxdbg_connect(readCheckBox_, &QCheckBox::stateChanged, [this](int state) {
        bool is_checked = state == 2;
        data_.is_stop_when_read = is_checked;
        this->updateState();
    });

    cxxdbg_connect(changedCheckBox_, &QCheckBox::stateChanged, [this](int state) {
        bool is_checked = state == 2;
        data_.is_stop_when_write = is_checked;
        this->updateState();
    });

    cxxdbg_connect(sizeEdit_, &QLineEdit::textChanged, [this](const QString & text) {
        data_.size = text;
        this->updateState();
    });

    cxxdbg_connect(okButton_, &QPushButton::clicked, [this, &bplist](auto && ...) {
        this->setStatus("Watchpoint is being added, please wait...");

        std::string size_str = data_.size.toStdString();
        auto size = static_cast<size_t>(std::atoi(size_str.c_str()));
        bool read = data_.is_stop_when_read;
        bool write = data_.is_stop_when_write;

        auto handler = [this](const cxxdbg::async::result<cxxdbg::dbg::watchpoint*> & res) {
            if (res.is_ok()) {
                this->accept();
            } else {
                QString status = QString::fromStdString(res.error());
                QMessageBox::critical(this, "Failed to add watchpoint", status);
                this->setStatus(status);
            }
        };

        if (data_.is_variable) {
            std::string description = data_.variable.toStdString();
            bplist.add_var_watchpoint(description, read, write, size, handler);
        } else if (data_.is_expression) {
            std::string description = data_.expression.toStdString();
            bplist.add_expr_watchpoint(description, read, write, size, handler);
        }
    });
}

namespace {
std::pair<bool, QString> validate(const AddWatchpointDialog::DialogData & data) {
    if (data.is_variable && data.variable.isEmpty()) {
        return {false, "variable name should not be empty"};
    }

    if (data.is_expression && data.expression.isEmpty()) {
        return {false, "address expression should not be empty"};
    }

    if (!data.is_stop_when_write && !data.is_stop_when_read) {
        return {false, "at least one stop condition should be selected"};
    }

    return {true, "ok"};
}
}

void AddWatchpointDialog::updateState() {
    assert(data_.is_variable != data_.is_expression && "one and only one option must be true");

    bool is_variable = data_.is_variable;
    bool is_expression = data_.is_expression;

    variableEdit_->setEnabled(is_variable);
    variableLabel_->setEnabled(is_variable);

    expressionEdit_->setEnabled(is_expression);
    expressionLabel_->setEnabled(is_expression);

    bool is_valid = false;
    QString status;

    std::tie(is_valid, status) = validate(data_);

    data_.status = status;
    setStatus(status);

    okButton_->setEnabled(is_valid);
}

void AddWatchpointDialog::initDialogState() {
    bool is_variable = data_.is_variable;
    bool is_expression = data_.is_expression;

    variableRadio_->setChecked(is_variable);
    expressionRadio_->setChecked(is_expression);

    readCheckBox_->setChecked(data_.is_stop_when_read);
    changedCheckBox_->setChecked(data_.is_stop_when_write);

    setStatus(data_.status);
}

void AddWatchpointDialog::setStatus(const QString & status) {
    statusLabel_->setText(tr("Status: ") + status);
}


}
