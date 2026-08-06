// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file CxxdbgDockWidget.cpp
/// Contains implementation of the CxxdbgDockWidget class.

#include "CxxdbgDockWidget.h"
#include <QAction>
#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QStyle>
#include <QToolBar>
#include <QVBoxLayout>


namespace cxxdbg::gui {


CxxdbgDockWidget::CxxdbgDockWidget(const QString & title, QWidget * parent, Qt::WindowFlags flags):
QDockWidget(title, parent, flags) {

    auto body = new QWidget;

    if (QApplication::style()->objectName() == "fusion") {
        auto p = QApplication::palette();
        auto col = p.mid().color();

        auto s = "QDockWidget::title { "
                 "  border-top: 1px solid %1;"
                 "  border-left: 1px solid %1;"
                 "  border-right: 1px solid %1;"
                 "  padding-top: 2px;s"
                 "}";
        setStyleSheet(QString{s}.arg(col.name()));

        auto s2 = "#dock_body { "
                  "     border-left: 1px solid %1;"
                  "     border-right: 1px solid %1;"
                  "     border-bottom: 1px solid %1;"
                  "}";
        body->setObjectName("dock_body");
        body->setStyleSheet(QString{s2}.arg(col.name()));
        body->setContentsMargins(1, 1, 1, 1);
    }

    QDockWidget::setWidget(body);

    layout_ = new QVBoxLayout{body};
    layout_->setContentsMargins({});
    layout_->setSpacing(0);
}


void CxxdbgDockWidget::setContentEnabled(bool value) {
    this->widget()->setEnabled(value);
}


void CxxdbgDockWidget::setWidget(QWidget * content) {
    // removing existing widget from layout
    auto idx = (toolBar_ != nullptr) ? 1 : 0;
    if (auto child = layout_->takeAt(idx)) {
        delete child->widget();
        delete child;
    }

    // adding widget to layout
    layout_->addWidget(content);
}


QToolBar * CxxdbgDockWidget::toolBar() {
    if (toolBar_ == nullptr) {
        toolBar_ = new QToolBar;
        layout_->insertWidget(0, toolBar_);
        toolBar_->setIconSize(QSize(16, 16));

        if (QApplication::style()->objectName() == "fusion") {
            toolBar_->setStyleSheet("padding: 1px;");
        }
    }

    return toolBar_;
}


}
