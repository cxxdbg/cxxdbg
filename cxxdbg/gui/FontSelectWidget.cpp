// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file FontSelectWidget.cpp
/// Contains implementation of the FontSelectWidget class.

#include "FontSelectWidget.h"
#include "cxxdbg_connect.hpp"
#include <QFontDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>


namespace cxxdbg::gui {


FontSelectWidget::FontSelectWidget(const QFont & font,
                                   const QFont & defaultFont,
                                   QWidget * parent):
QWidget(parent),
selectedFont_{font},
defaultFont_(defaultFont) {

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setContentsMargins({});

    // font preview

    preview_ = new QLineEdit(this);
    layout->addWidget(preview_);
    preview_->setReadOnly(true);
    preview_->setFixedHeight(50);
    preview_->setMinimumWidth(QFontMetrics(this->font()).horizontalAdvance(QString(40, '0')));
    preview_->setAlignment(Qt::AlignCenter);

    // buttons

    QHBoxLayout * buttonsLayout = new QHBoxLayout;
    layout->addLayout(buttonsLayout);

    QPushButton * selectButton = new QPushButton(tr("Select..."));
    buttonsLayout->addWidget(selectButton);
    cxxdbg_connect(selectButton, &QPushButton::clicked, [this](auto && ...) {
        bool ok = false;
        QFont newFont = QFontDialog::getFont(&ok, selectedFont_);
        if (ok) {
            selectFont(newFont);
            emit changed();
        }
    });

    resetButton_ = new QPushButton(tr("Reset"));
    buttonsLayout->addWidget(resetButton_);
    cxxdbg_connect(resetButton_, &QPushButton::clicked, [this](auto && ...) {
        selectFont(defaultFont_);
        emit changed();
    });

    buttonsLayout->addStretch(1);

    selectFont(font);
}


QFont FontSelectWidget::selectedFont() const {
    return selectedFont_;
}


void FontSelectWidget::selectFont(const QFont & font) {
    selectedFont_ = font;
    preview_->setFont(font);
    
    QString fontName = QString("%1 %2pt")
            .arg(font.family())
            .arg(font.pointSize());

    preview_->setText(fontName);

    resetButton_->setEnabled(selectedFont() != defaultFont_);
}


}
