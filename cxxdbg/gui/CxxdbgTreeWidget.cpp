// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file CxxdbgTreeWidget.cpp
/// Contains implementation of CxxdbgTreeWidget class.

#include "CxxdbgTreeWidget.h"
#include "cxxdbg_connect.hpp"
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QSettings>
#include <sstream>


namespace cxxdbg::gui {


CxxdbgTreeWidget::CxxdbgTreeWidget(const QString & nm, QWidget * parent):
QTreeWidget(parent),
name_(nm) {
    restoreState();
}


void CxxdbgTreeWidget::addColumnSelectContextMenu() {
    for (int i = 1, e = headerItem()->columnCount(); i < e; ++i) {
        QString columnName = headerItem()->text(i);

        QAction * columnAction = new QAction(columnName, this);
        columnAction->setCheckable(true);
        columnAction->setChecked(!isColumnHidden(i));
        cxxdbg_connect(columnAction, &QAction::triggered, [this, i](bool checked) {
            if (checked) {
                showColumn(i);
            } else {
                hideColumn(i);
            }
        });

        header()->addAction(columnAction);
    }

    header()->setContextMenuPolicy(Qt::ActionsContextMenu);
}


void CxxdbgTreeWidget::saveState() {
    QSettings settings;
    settings.setValue(name_, header()->saveState());
}


void CxxdbgTreeWidget::restoreState() {
    QSettings settings;
    QVariant val = settings.value(name_);
    if (val.isNull())
        return;

    header()->restoreState(val.toByteArray());
}


void CxxdbgTreeWidget::copySelectedToClipboard() {
    std::wostringstream str;

    QList<QTreeWidgetItem*> items = selectedItems();
    for (auto it = items.begin(), end = items.end(); it != end; ++it) {
        QTreeWidgetItem * item = *it;
        bool first = true;
        for (int i = 0, e = item->columnCount(); i < e; ++i) {
            if (first) {
                first = false;
            } else {
                str << "\t";
            }

            str << item->text(i).toStdWString();
        }

        str << "\n";
    }

    QApplication::clipboard()->setText(QString::fromStdWString(str.str()));
}

void CxxdbgTreeWidget::addSeparator() {
    QAction * sep = new QAction{this};
    sep->setSeparator(true);
    addAction(sep);
}


}
