// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <QDialog>


namespace cxxdbg::dbg {
    class breakpoint_list;
}


class QLineEdit;
class QLabel;
class QDialogButtonBox;
class QPushButton;
class QCheckBox;
class QRadioButton;


namespace cxxdbg::gui {


class AddWatchpointDialog: public QDialog {
    Q_OBJECT;
public:
    struct DialogData {
        bool is_stop_when_read = false; ///< flag which means that watchpoint will stop when value is read
        bool is_stop_when_write = true; ///< flag which means that watchpoint will stop when value is changed

        bool is_expression = false; ///< flag which means that watchpoint is specified by expression address
        bool is_variable = true;    ///< flag which means that watchpoint is specified by variable name

        QString expression;         ///< Address expressino
        QString variable;           ///< Variable name

        QString size;               ///< Size value

        bool is_valid = false;      ///< flag means that dialog input is valid

        QString status = QString::fromStdString("Status: variable name is not valid");
    };

    explicit AddWatchpointDialog(cxxdbg::dbg::breakpoint_list & bplist, QWidget * parent= nullptr);

    /// Sets status
    void setStatus(const QString & status);

private:
    /// Initializes dialog state
    void initDialogState();

    /// Updates state of gui items
    void updateState();

    DialogData data_;   ///< Dialog data

    QRadioButton * variableRadio_{};    ///< Radio button for specifying watchpoint by variable name
    QLineEdit * variableEdit_{};        ///< Variable name line edit
    QLabel * variableLabel_{};          ///< Variable name option label

    QRadioButton * expressionRadio_{};  ///< Radio button for specifying watchpoing by expresion address
    QLineEdit * expressionEdit_{};      ///< Expression address line edit
    QLabel * expressionLabel_{};        ///< Expression address option label

    QCheckBox * readCheckBox_{};        ///< Check box for specifying is_stop_when_read flag
    QCheckBox * changedCheckBox_{};     ///< Check box for specifying is_stop_when_write flag

    QLineEdit * sizeEdit_{};            ///< Size line edit

    QLabel * statusLabel_{};            ///< Status label

    QDialogButtonBox * buttons_{};      ///< Dialog buttons Ok and Cancel
    QPushButton * okButton_{};          ///< Ok button from button box
};


}
