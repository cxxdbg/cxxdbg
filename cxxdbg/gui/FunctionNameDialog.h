// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <QDialog>

class QPushButton;
class QLineEdit;
class QLabel;


namespace cxxdbg::gui {


class FunctionNameDialog: public QDialog {
Q_OBJECT;
public:
    explicit FunctionNameDialog(QWidget * parent  = nullptr);

    QString getFunctionName();

    QLineEdit * functionNameEdit_;
    QPushButton * okButton_;
    QLabel * statusLabel_;

public slots:
    void updateState();

private:
    std::pair<bool, QString> validate();

    QString functionName_;
};


}
