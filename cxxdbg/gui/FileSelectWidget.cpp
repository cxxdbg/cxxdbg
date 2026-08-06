// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "FileSelectWidget.h"
#include "cxxdbg_connect.hpp"
#include <QApplication>
#include <QFileDialog>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>


namespace cxxdbg::gui {


FileSelectWidget::FileSelectWidget(const QString &dialogCaption,
                                   bool dir,
                                   QWidget *parent) :
QWidget(parent)
{
    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->setContentsMargins({});

    fileName_ = new QLineEdit(this);
    QFontMetrics metrics(QApplication::font());
    fileName_->setMinimumWidth(metrics.horizontalAdvance(QString(40, '0')));
    layout->addWidget(fileName_);
    cxxdbg_connect(fileName_, &QLineEdit::textChanged, [this](auto && ...) { emit changed(); });

    browseButton_ = new QPushButton(tr("Browse") + "...", this);
    layout->addWidget(browseButton_);
    browseButton_->setWhatsThis(tr("Opens file select dialog box"));
    cxxdbg_connect(browseButton_, &QPushButton::clicked, [this, dialogCaption, dir](auto && ...) {

        QString fileName;
        if (dir) {
            fileName = QFileDialog::getExistingDirectory(this, dialogCaption);
        } else {
            fileName = QFileDialog::getOpenFileName(this, dialogCaption);
        }

        if (fileName.isEmpty())
            return;

        // change file name in line edit
        fileName_->setText(fileName);
    });
}


void FileSelectWidget::setFileName(const QString & name) {
    fileName_->setText(name);
}


QString FileSelectWidget::fileName() const {
    return fileName_->text();
}


void FileSelectWidget::setBrowseButtonEnabled(bool value) {
    browseButton_->setEnabled(value);
}


void FileSelectWidget::setPlaceholderText(const QString & str) {
    fileName_->setPlaceholderText(str);
}


}
