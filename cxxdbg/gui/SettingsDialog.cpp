// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file SettingsDialog.cpp
/// Contains implementation of the SettingsDialog class.

#include "SettingsDialog.h"
#include "cxxdbg_connect.hpp"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QVBoxLayout>


namespace cxxdbg::gui {


SettingsDialog::SettingsDialog(bool displayRestore, QWidget * parent):
QDialog{parent} {
    // dialog layout
    auto layout = new QVBoxLayout(this);

    // content layout
    auto contentLayout = new QHBoxLayout;
    layout->addLayout(contentLayout);
    contentLayout->setContentsMargins({});

    menu_ = new QTreeWidget;
    contentLayout->addWidget(menu_);
    auto szPol = menu_->sizePolicy();
    szPol.setHorizontalPolicy(QSizePolicy::Minimum);
    menu_->setSizePolicy(szPol);
    menu_->setHeaderHidden(true);

    stack_ = new QStackedWidget;
    contentLayout->addWidget(stack_);

    cxxdbg_connect(menu_, &QTreeWidget::currentItemChanged, [this](QTreeWidgetItem * curr, auto prev) {
        auto pageIdx = curr->data(0, Qt::UserRole).toInt();
        stack_->setCurrentIndex(pageIdx);
    });

    menu_->setCurrentItem(menu_->topLevelItem(0));

    // dialog buttons
    auto btns = QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply;
    if (displayRestore) {
        btns |= QDialogButtonBox::RestoreDefaults;
    }
    buttons_ = new QDialogButtonBox(btns);
    layout->addWidget(buttons_);
    cxxdbg_connect(buttons_, &QDialogButtonBox::accepted, [this] {
        emit applyClicked();
        accept();
    });

    cxxdbg_connect(buttons_, &QDialogButtonBox::rejected, [this] { reject(); });
    cxxdbg_connect(buttons_->button(QDialogButtonBox::Apply), &QPushButton::clicked, [this] { emit applyClicked(); });
    cxxdbg_connect(buttons_->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, [this] { emit restoreDefaultsClicked(); });
}


QTreeWidgetItem * SettingsDialog::addPage(const QString & name, QWidget * page, QTreeWidgetItem * parent) {
    auto pageContainer = new QWidget;
    auto pageContainerLayout = new QVBoxLayout{pageContainer};
    pageContainerLayout->setContentsMargins({});
    pageContainerLayout->addWidget(page);
    pageContainerLayout->addStretch(1);
    auto idx = stack_->addWidget(pageContainer);
    
    QTreeWidgetItem * item = new QTreeWidgetItem{{name}};
    item->setData(0, Qt::UserRole, QVariant{idx});

    if (parent != nullptr) {
        parent->addChild(item);
    } else {
        menu_->addTopLevelItem(item);
    }

    return item;
}

void SettingsDialog::setApplyEnabled(bool enabled) {
    buttons_->button(QDialogButtonBox::Apply)->setEnabled(enabled);
}


}
