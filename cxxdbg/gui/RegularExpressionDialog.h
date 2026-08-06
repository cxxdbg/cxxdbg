// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>


namespace cxxdbg::gui {


class RegularExpressionDialog : public QDialog
{
    Q_OBJECT

    public:
        RegularExpressionDialog(const QString & expression,
                                QWidget *       parent = 0);

        const QString & expression()
        {
            return expression_;
        }

    public slots:
        void onTextChanged(const QString &);

    private:
        void updateState(const QString & expression);

        QLineEdit *                   expressionEdit_;
        QLabel *                      correctnessLabel_;
        QLabel *                      statusBarLabel_;
        QPushButton *                 okButton_;
        QPixmap                       rightPixmap_;
        QPixmap                       wrongPixmap_;
        QString                       expression_;
};


}
