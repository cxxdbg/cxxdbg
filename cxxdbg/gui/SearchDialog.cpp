// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file SearchDialog.cpp
/// Contains implementation of the SearchDialog class.

#include "SearchDialog.h"
#include "document_list_widget.hpp"
#include "FormLayout.h"
#include "MainWindow.h"
#include "cxxdbg_connect.hpp"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>


namespace cxxdbg::gui {


SearchDialog::SearchDialog(document_list_widget * docsWidget, QWidget * parent):
docsWidget_{docsWidget},
QDialog(parent) {

    setWindowTitle(tr("Find"));

    QVBoxLayout * layout = new QVBoxLayout(this);

    auto formLayout = new FormLayout;
    layout->addLayout(formLayout);

    // search string
    searchStrEdit_ = new QLineEdit(this);
    formLayout->addRow(tr("Search string:"), searchStrEdit_);
    cxxdbg_connect(searchStrEdit_, &QLineEdit::textChanged, [this](auto && ...) { onSearchCondChanged(); });

    // match case check box
    matchCaseCheckBox_ = new QCheckBox(tr("Match case"), this);
    formLayout->addRow(matchCaseCheckBox_);
    cxxdbg_connect(matchCaseCheckBox_, &QCheckBox::toggled, [this](auto && ...) { onSearchCondChanged(); });

    // dialog buttons
    QDialogButtonBox * buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);
    cxxdbg_connect(buttons, &QDialogButtonBox::rejected, [this] { reject(); });

    findNextButton_ = buttons->addButton(tr("Find next"), QDialogButtonBox::ActionRole);
    cxxdbg_connect(findNextButton_, &QPushButton::clicked, [this](auto && ...) {
        docsWidget_->searchNext();
    });

    findPrevButton_ = buttons->addButton(tr("Find previous"), QDialogButtonBox::ActionRole);
    cxxdbg_connect(findPrevButton_, &QPushButton::clicked, [this](auto && ...) {
        docsWidget_->searchPrev();
    });

    setFixedHeight(sizeHint().height());

    // updating search buttons state when canDoSearch property of the main
    // window changes
    cxxdbg_connect(docsWidget_, &document_list_widget::canDoSearchChanged, [this] {updateSearchButtons(); });

    // remove highlight on close
    cxxdbg_connect(this, &QDialog::rejected, [this] { docsWidget_->removeSearchStringHighlight(); });

    updateSearchButtons();
}


void SearchDialog::showEvent(QShowEvent * event) {
    searchStrEdit_->setFocus();
    onSearchCondChanged();

    QDialog::showEvent(event);
}


void SearchDialog::updateSearchButtons() {
    bool enable = docsWidget_->canDoSearch();

    findNextButton_->setEnabled(enable);
    findPrevButton_->setEnabled(enable);
}


void SearchDialog::onSearchCondChanged() {
    // highlighting search text in documents widget
    // highlightSearchString will notify us about changing ability to
    // search next / prev via canDoSearchChanged signal
    docsWidget_->highlightSearchString(searchStrEdit_->text(),
                                       matchCaseCheckBox_->isChecked());
}


}
