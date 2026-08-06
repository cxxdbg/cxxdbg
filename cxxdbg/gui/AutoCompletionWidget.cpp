// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "AutoCompletionWidget.hpp"

#include "CxxdbgTreeView.h"
#include "Utils.h"
#include "cxxdbg_connect.hpp"

#include <QFontDatabase>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QItemSelectionModel>
#include <QFontMetrics>
#include <QScrollBar>

#include <iostream>


namespace cxxdbg::gui {


AutoCompletionWidget::AutoCompletionWidget(QAbstractItemModel * model, QWidget * parent):
        QWidget{parent, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint},
        model_{model},
        max_visible_rows_{max_visible_rows_def_},
        max_chars_per_line_{max_chars_per_line_def_},
        max_items_check_{100} {

    assert(parent != nullptr && "parent must be given");

    setWindowModality(Qt::WindowModality::WindowModal);
    setContentsMargins(0, 0, 0, 0);

    auto * lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    this->setLayout(lay);

    view_ = new CxxdbgTreeView(tr("available commands"), this);
    lay->addWidget(view_);
    view_->setContextMenuPolicy(Qt::DefaultContextMenu);

    assert(model != nullptr);

    view_->setModel(model);
    view_->header()->hide();

    view_->setFont(defaultFixedFont());

    setSizePolicy(QSizePolicy(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Expanding));

    setFocusPolicy(Qt::FocusPolicy::NoFocus);
    view_->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    view_->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    view_->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

    view_->setRootIsDecorated(false);
    view_->setContentsMargins(0, 0, 0, 0);

    view_->setUniformRowHeights(true);
    view_->setTextElideMode(Qt::TextElideMode::ElideRight);

    // if we don't set ScrollBarAlwaysOff, white bar will always cover the lowest row
    // even if scrollbar is hidden
    view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    installEventFilter(this);

    cxxdbg_connect(view_, &CxxdbgTreeView::onDoubleClicked, [this] {
        auto index = getSelectedIndex();
        if (index.isValid())
            emit itemDoubleClicked(index);
    });
}


void AutoCompletionWidget::showWidget() {
    auto geometry = parentWidget()->geometry();
    resize(geometry.width(), geometry.height());
    show();
}


void AutoCompletionWidget::showWidget(const QPoint & pos, Placement placement, int spacing) {
    assert (parentWidget() != nullptr);

    const auto loc = parentWidget()->mapFromGlobal(pos);

    auto metrics = QFontMetrics(view_->font());

    int max_chars = 0;

    auto num_items = model_->rowCount(view_->rootIndex());
    auto max_iter = std::min(num_items, max_model_items_check());

    auto max_advance = 0; // max advance of text in pixels

    for (int row = 0; row < max_iter; ++row) {
        auto index = model_->index(row, 0, view_->rootIndex());
        QString text = model_->data(index, Qt::DisplayRole).toString();
        int size = std::min(text.size(), static_cast<QString::size_type>(max_chars_per_line()));
        if (size > max_chars) {
            max_chars = size;
        }

        int advance = metrics.horizontalAdvance(text, size);
        if (advance > max_advance)
            max_advance = advance;
    }

    auto width = max_advance;

    int num_rows_visible = std::min(max_visible_rows(), num_items);
    auto height = num_rows_visible * metrics.height();

    // offset
    width += view_->getHorizontalOffset();
    height += view_->getVerticalOffset();

    // frames
    width += 2 * view_->frameWidth();
    height += 2 * view_->frameWidth();

    // magic
    width += metrics.maxWidth() * 3 / 4;
    height += metrics.xHeight() / 4;

    // scrollbars are problematic
    auto * horizontalScrollBar = view_->horizontalScrollBar();
    if (horizontalScrollBar->isVisible())   // maybe some day we will need it
        height += horizontalScrollBar->height();

    auto * verticalScrollBar = view_->verticalScrollBar();

    bool is_vertical_scrollbar_visible = num_items > max_visible_rows();
    // workaround for white bar hiding item text
    view_->setVerticalScrollBarPolicy(is_vertical_scrollbar_visible ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);

    if (is_vertical_scrollbar_visible)     // can be visible if num rows is > max_rows
        width += verticalScrollBar->width();

    resize(width, height);

    switch (placement) {
        case Above:
            move(loc.x(), loc.y() - height - spacing);
            break;
        case Below:
            move(loc.x(), loc.y() + spacing);
            break;
        default:
            assert(false && "unknown placement type");
    }

    show();

#if 0
    std::cout << "loc = " << loc.x() << ", " << loc.y() << std::endl;
    std::cout << "pos = " << pos.x() << ", " << pos.y() << std::endl;

    std::cout << "width = " << width << std::endl;
    std::cout << "height = " << height << std::endl;
#endif
}


void AutoCompletionWidget::hideWidget() {
    hide();
}


QModelIndex AutoCompletionWidget::getSelectedIndex() {
    auto indexes = view_->selectionModel()->selectedIndexes();

    if (indexes.size() == 1)
        return indexes.front();

    return {};
}


bool AutoCompletionWidget::eventFilter(QObject * obj, QEvent * event) {
    if (event->type() == QEvent::FocusOut) {
        hideWidget();
    }

    if (event->type() == QEvent::KeyPress) {
        auto * keyEvent = (QKeyEvent *)event;
        if (keyEvent->key() == Qt::Key_Escape) {
            hideWidget();
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}


}
