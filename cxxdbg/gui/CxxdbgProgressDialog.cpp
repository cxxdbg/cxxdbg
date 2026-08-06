// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file CxxdbgProgressDialog.cpp
/// Contains implementation of the CxxdbgProgressDialog class.

#include "CxxdbgProgressDialog.h"
#include <QKeyEvent>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>


namespace cxxdbg::gui {


CxxdbgProgressDialog::CxxdbgProgressDialog(const QString & title,
                                       const QString & text,
                                       int min,
                                       int max,
                                       QWidget * parent):
QDialog{parent} {
    setWindowTitle(title);
    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    layout->addWidget(new QLabel(text));

    progress_ = new QProgressBar{};
    progress_->setRange(min, max);
    layout->addWidget(progress_);
}


CxxdbgProgressDialog::~CxxdbgProgressDialog() {
}


void CxxdbgProgressDialog::show() {
    exec();
}


void CxxdbgProgressDialog::close() {
    accept();
}


void CxxdbgProgressDialog::keyPressEvent(QKeyEvent * event) {
    // ignore escape
    if (event->key() != Qt::Key_Escape) {
        QDialog::keyPressEvent(event);
    } else {
        event->ignore();
    }
}


void CxxdbgProgressDialog::closeEvent(QCloseEvent * event) {
    // dialog is not closable
    event->ignore();
}


}
