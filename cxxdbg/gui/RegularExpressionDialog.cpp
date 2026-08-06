// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "RegularExpressionDialog.h"
#include "ui_RegularExpressionDialog.h"
#include "cxxdbg_connect.hpp"

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>

#include <cassert>

#include "cxxdbg/dbg/core/regex.hpp"


namespace cxxdbg::gui {


#ifndef UI_REGULAREXPRESSIONDIALOG_H

// stub for IDE
namespace Ui
{
class RegularExpressionDialog
{
    public:
        template<class T>
        void setupUi(T * t)
        {
        }
};
}
#endif

RegularExpressionDialog::RegularExpressionDialog(const QString & expression,
        QWidget *                                                parent)
        : QDialog(parent),
          expression_(expression)
{
    Ui::RegularExpressionDialog ui;
    ui.setupUi(this);
    
    setWindowTitle(tr("Edit regular expression"));

    rightPixmap_      = QIcon::fromTheme("dialog-information").pixmap(16, 16);
    wrongPixmap_      = QIcon::fromTheme("dialog-error").pixmap(16, 16);
    expressionEdit_   = findChild<QLineEdit *>("regularExpressionEdit");
    correctnessLabel_ = findChild<QLabel *>("correctnessLabel");
    statusBarLabel_   = findChild<QLabel *>("statusLabel");

    assert((expressionEdit_ != 0) && "Failed to find expression edit field");
    assert((correctnessLabel_ != 0) && "Failed to find correctness Label");
    assert((statusBarLabel_ != 0) && "Failed to find statusbar label");
    
    cxxdbg_connect(expressionEdit_, &QLineEdit::textChanged, [this](const QString & text) { onTextChanged(text); });

    QDialogButtonBox * buttonBox = findChild<QDialogButtonBox *>();

    assert((buttonBox != 0) && "cannot find dialog button box");

    okButton_ = buttonBox -> button(QDialogButtonBox::Ok);

    expressionEdit_ -> setText(expression);
    
    updateState(expression);
}

void RegularExpressionDialog::onTextChanged(const QString & text)
{
    expression_ = text;

    updateState(text);
}

void RegularExpressionDialog::updateState(const QString & expression)
{
    // validate expression
    std::string result    = cxxdbg::dbg::core::validate_regex(expression.toStdString());
    bool        isCorrect = result.length() == 0;

    okButton_ -> setEnabled(isCorrect);

    QString statusText = isCorrect ? tr("Enter regular expression") : QString::fromStdString(result);

    statusBarLabel_ -> setText(statusText);

    const QPixmap & pixmap = isCorrect ? rightPixmap_ : wrongPixmap_;

    correctnessLabel_ -> setPixmap(pixmap);
}


}
