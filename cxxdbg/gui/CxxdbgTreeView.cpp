// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file CxxdbgTreeView.cpp
/// Contains implementation of the CxxdbgTreeView class.

#include "CxxdbgTreeView.h"
#include "CxxdbgTreeWidget.h"
#include "cxxdbg_connect.hpp"
#include "cxxdbg/gui/AppItemModel.h"
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QSettings>
#include <QStyle>
#include <QDebug>

#include <sstream>


namespace cxxdbg::gui {


class CxxdbgTreeViewHeader: public QHeaderView {
public:
    CxxdbgTreeViewHeader(CxxdbgTreeView * tView, Qt::Orientation o):
    QHeaderView(o, tView), treeView_{tView} {}

protected:
    void mousePressEvent(QMouseEvent * event) override {
        if (treeView_->onHeaderMousePressEvent(event)) {
            QHeaderView::mousePressEvent(event);
        } else {
            event->ignore();
        }
    }

    void mouseReleaseEvent(QMouseEvent * event) override {
        if (treeView_->onHeaderMouseReleaseEvent(event)) {
            QHeaderView::mouseReleaseEvent(event);
        } else {
            event->ignore();
        }
    }

private:
    /// Pointer to tree view
    CxxdbgTreeView * treeView_;
};


CxxdbgTreeView::CxxdbgTreeView(const QString & nm, QWidget * parent):
QTreeView{parent},
name_(nm) {
    setHeader(new CxxdbgTreeViewHeader(this, Qt::Horizontal));
    header()->setStretchLastSection(true);

    // enable sorting after load if sorting indicator was shown
    if (header()->isSortIndicatorShown()) {
        setSortingEnabled(true);
    }

    setSelectionMode(QAbstractItemView::ContiguousSelection);
    updateColumnsContextMenu();

    // adding select all / copy context menu actions

    // copy action
    copyAction_ = new QAction(QIcon::fromTheme("edit-copy"), tr("Copy"), this);
    copyAction_->setEnabled(false);
    addAction(copyAction_);
    cxxdbg_connect(copyAction_, &QAction::triggered, [this] {
        copySelectedToClipboard();
    });

    // select all action
    QAction * selectAll = new QAction(QIcon::fromTheme("edit-select-all"), tr("Select all"), this);
    addAction(selectAll);
    cxxdbg_connect(selectAll, &QAction::triggered, [this]() { this->selectAll(); });

    setContextMenuPolicy(Qt::ActionsContextMenu);
}


void CxxdbgTreeView::saveState() {
    QSettings settings;
    settings.setValue(name_, header()->saveState());
}


void CxxdbgTreeView::restoreState() {
    QSettings settings;
    QVariant val = settings.value(name_);
    if (val.isNull())
        return;

    header()->restoreState(val.toByteArray());
    columnsEqualSize_ = false;
}


void CxxdbgTreeView::copySelectedToClipboard() {
    std::wostringstream str;

    QModelIndexList items = selectionModel()->selectedRows();

    for (auto it = items.begin(), end = items.end(); it != end; ++it) {
        QModelIndex idx = *it;

        bool first = true;
        for (int i = 0, e = model()->columnCount(); i < e; ++i) {
            if (first) {
                first = false;
            } else {
                str << "\t";
            }

            str << model()->data(idx.sibling(idx.row(), i)).toString().toStdWString();
        }

        str << "\n";
    }

    QApplication::clipboard()->setText(QString::fromStdWString(str.str()));
}


void CxxdbgTreeView::setModel(QAbstractItemModel * model) {
    QTreeView::setModel(model);
    updateColumnsContextMenu();

    // enable copy action only if something is selected
    cxxdbg_connect(selectionModel(), &QItemSelectionModel::selectionChanged, [this] {
        copyAction_->setEnabled(!selectionModel()->selectedRows().empty());
    });

    auto * expandState = dynamic_cast<ExpandStateSupport *>(model);
    if (expandState && expandState->hasExpandStateSupport()) {
        disconnect(dataChangedConnection_);
        dataChangedConnection_ =
                cxxdbg_connect(model, &QAbstractItemModel::dataChanged,
                        [this, expandState] (const QModelIndex & first, const QModelIndex & last, auto && ...) {
                            if (expandState->hasExpandStateSupport()) {
                                auto state = expandState->getExpandState(first);
                                switch (state) {
                                    case ExpandStateSupport::ExpandState::expanded:
                                        if (!this->isExpanded(first))
                                            this->expand(first);
                                        break;
                                    case ExpandStateSupport::ExpandState::collapsed:
                                        if (this->isExpanded(first))
                                            this->collapse(first);
                                        break;
                                    default:
                                        break;
                                }
                             }
                        });

        disconnect(nodeCollapsedConnection_);
        nodeCollapsedConnection_ = connect(this, &QTreeView::collapsed, [expandState](const QModelIndex & index) {
            if (expandState->getExpandState(index) == ExpandStateSupport::ExpandState::collapsed) {
                return;
            }

            expandState->setExpandState(index, ExpandStateSupport::ExpandState::collapsed);
        });

        cxxdbg_connect(this, &QTreeView::expanded, [expandState](const QModelIndex & index) {
            if (expandState->getExpandState(index) == ExpandStateSupport::ExpandState::expanded) {
                return;
            }

            expandState->setExpandState(index, ExpandStateSupport::ExpandState::expanded);
        });
    }

    if (auto mdl = dynamic_cast<AppRoItemModel*>(model)) {
        cxxdbg_connect(mdl, &AppRoItemModel::expandRow, [this](const QModelIndex& index) {
            this->expand(index);
        });
    }
}


void CxxdbgTreeView::setModel(SortableAppRoItemModel * smodel) {
    sortableModel_ = smodel;
    setModel(static_cast<QAbstractItemModel*>(smodel));

    // enable sorting. We manually handle click on header view and
    // change sort order. This is needed for disabling sorting
    // after clicking to header
    srtEnabled_ = true;
}


void CxxdbgTreeView::setSectionSize(int index, int size) {
    columnsEqualSize_ = false;
    header()->resizeSection(index, size);
}


void CxxdbgTreeView::addSeparator() {
    QAction * sep = new QAction{this};
    sep->setSeparator(true);
    addAction(sep);
}


int CxxdbgTreeView::getHorizontalOffset() const {
    return horizontalOffset();
}


int CxxdbgTreeView::getVerticalOffset() const {
    return verticalOffset();
}


void CxxdbgTreeView::setDisplayLinesBetweenRows(bool val) {
    displayLinesBetweenRows_ = val;
    update();
}


void CxxdbgTreeView::setColumnsEqualSize(bool val) {
    columnsEqualSize_ = val;
}


void CxxdbgTreeView::setColumnHidden(int idx, bool isHidden) {
    QTreeView::setColumnHidden(idx, isHidden);
    updateColumnsContextMenu();
}


void CxxdbgTreeView::drawRow(QPainter * painter,
                           const QStyleOptionViewItem & option,
                           const QModelIndex & index) const {

    if (displayLinesBetweenRows_) {
        painter->save();
        painter->setPen(QColor(Qt::lightGray));
        painter->drawLine(option.rect.left(), option.rect.bottom(),
                          option.rect.right(), option.rect.bottom());
        painter->restore();
    }

    QTreeView::drawRow(painter, option, index);
}


void CxxdbgTreeView::resizeEvent(QResizeEvent * event) {
    // ignore event if:
    // - auto resize is disabled
    // - size is negative
    // - column count is 0 or 1
    if (!columnsEqualSize_ ||
        event->size().width() < 0 ||
        header()->count() < 2) {

        QTreeView::resizeEvent(event);
        return;
    }

    auto sz = static_cast<float>(width()) / header()->count();
    for (int i = 0; i < header()->count() - 1; ++i) {
        header()->resizeSection(i, static_cast<int>(sz));
    }

    QTreeView::resizeEvent(event);
}


void CxxdbgTreeView::updateColumnsContextMenu() {

    if (model() == nullptr)
        return;

    // removing all actions
    QList<QAction*> actions = header()->actions();
    for (auto action : actions) {
        header()->removeAction(action);
    }

    // adding new action for each column
    for (int i = 1, e = model()->columnCount(); i < e; ++i) {
        QString columnName = model()->headerData(i, Qt::Horizontal).toString();

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

    // setting context menu policy
    header()->setContextMenuPolicy(Qt::ActionsContextMenu);
}


/// Returns splitter width from style
static int getSplitterWidth(QWidget * widget) {
    int splitterWidth = widget->style()->pixelMetric(QStyle::PM_SplitterWidth);

    // if splitter width is less than 6 then we should still use 6
    if (splitterWidth < 6) {
        splitterWidth = 6;
    }

    return splitterWidth;
}


bool CxxdbgTreeView::onHeaderMousePressEvent(QMouseEvent * event) {
    if (event->button() != Qt::LeftButton) {
        return false;
    }

    columnsEqualSize_ = false;

    if (!srtEnabled_) {
        return true;
    }

    auto idx = this->header()->logicalIndexAt(event->pos());

    // checking if user presses / releases on column splitter
    int splitterWidth = getSplitterWidth(this);
    int sectionStart = this->header()->sectionViewportPosition(idx);
    int sectionEnd = sectionStart + this->header()->sectionSize(idx);

    if (std::abs(sectionStart - event->pos().x()) <= splitterWidth / 2 ||
        std::abs(sectionEnd - event->pos().x()) <= splitterWidth / 2) {
        return true;
    }

    if (srtEnabled_ && !isSortingEnabled()) {
        // no current sorting. Enable sorting and set it to ascending
        header()->setSortIndicator(idx, Qt::AscendingOrder);
        setSortingEnabled(true);
        skipNextHeaderRelease_ = true;
        return false;
    }

    // disable sorting after clicking to column with descending order
    if (header()->isSortIndicatorShown() &&
        header()->sortIndicatorSection() == idx &&
        header()->sortIndicatorOrder() == Qt::DescendingOrder) {

        setSortingEnabled(false);
        skipNextHeaderRelease_ = true;
        sortableModel_->disableSorting();
        return false;
    }

    return true;
}


bool CxxdbgTreeView::onHeaderMouseReleaseEvent(QMouseEvent * event) {
    if (event->button() != Qt::LeftButton) {
        return false;
    }

    columnsEqualSize_ = false;

    if (!srtEnabled_) {
        return true;
    }

    auto idx = this->header()->logicalIndexAt(event->pos());

    // checking if user presses / releases on column splitter
    int splitterWidth = getSplitterWidth(this);
    int sectionStart = this->header()->sectionViewportPosition(idx);
    int sectionEnd = sectionStart + this->header()->sectionSize(idx);
    if (std::abs(sectionStart - event->pos().x()) <= splitterWidth / 2 ||
        std::abs(sectionEnd - event->pos().x()) <= splitterWidth / 2) {
        return true;
    }

    if (skipNextHeaderRelease_) {
        skipNextHeaderRelease_ = false;
        return false;
    }

    return true;
}

void CxxdbgTreeView::mouseDoubleClickEvent(QMouseEvent * event) {
    emit onDoubleClicked();

    QTreeView::mouseDoubleClickEvent(event);
}


}
