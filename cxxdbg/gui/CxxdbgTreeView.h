// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file CxxdbgTreeView.h
/// Contains definition of the CxxdbgTreeView class.

#pragma once

#include <QTreeView>
#include <QMetaObject>
#include <vector>

#include "AppItemModel.h"


namespace cxxdbg::gui {

class CxxdbgTreeViewHeader;


/// Tree view with extended functionality like selecting visible columns
/// and saving/restoring state
class CxxdbgTreeView: public QTreeView {
    Q_OBJECT;
    friend class CxxdbgTreeViewHeader;

public:
    /// Constructor, makes tree widget with specified name and parent
    CxxdbgTreeView(const QString & nm, QWidget * parent = nullptr);

    /// Saves state to settings
    void saveState();

    /// Restores state from settings
    void restoreState();

    /// Copies selected items to clipboard
    void copySelectedToClipboard();

    /// Sets tree view model
    void setModel(QAbstractItemModel * model) override;

    /// Sets sortable tree view model
    void setModel(SortableAppRoItemModel * smodel);
    
    /// sets section size
    void setSectionSize(int index, int size);

    /// Adds to context menu
    void addSeparator();

    /// Returns horizontal offset of the view
    int getHorizontalOffset() const;

    /// Returns vertical offset of the view
    int getVerticalOffset() const;

    /// Sets flag for displaying lines between rows
    void setDisplayLinesBetweenRows(bool val);

    /// Sets auto resize of columns to be of equal size
    void setColumnsEqualSize(bool val);

    /// Sets columnt hidden flag
    void setColumnHidden(int idx, bool isHidden);

signals:
    void onDoubleClicked();

protected:
    void drawRow(QPainter * painter,
                 const QStyleOptionViewItem & option,
                 const QModelIndex & index) const override;

    /// Called after widget resized
    void resizeEvent(QResizeEvent * event) override;

private:
    /// Updates context menu for selecting visible columns.
    /// Returns true if event should be passed to QHeaderView
    void updateColumnsContextMenu();

    /// Called when user presses mouse on header view.
    /// Returns true if event should be passed to QHeaderView
    bool onHeaderMousePressEvent(QMouseEvent * event);

    /// Called when user releases mouse on header view
    bool onHeaderMouseReleaseEvent(QMouseEvent * event);

    /// Called when user doubleclicks an item
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    QString name_;              ///< Widget name, used for saving/restoring state

    /// Pointer to sortable model
    SortableAppRoItemModel *sortableModel_ = nullptr;

    QAction * copyAction_;      ///< Copy context menu action
    bool srtEnabled_ = false;   ///< Is sorting enabled

    /// Is next header mouse release event should be skipped?
    bool skipNextHeaderRelease_ = false;

    /// Should lines between rows be displayed
    bool displayLinesBetweenRows_ = false;

    /// If true then columns will be resized after widget resize and will
    /// have equal size
    bool columnsEqualSize_ = true;

    /// dataChanged connection keeper
    QMetaObject::Connection dataChangedConnection_;

    /// nodeCollapsed connection keeper
    QMetaObject::Connection nodeCollapsedConnection_;
};


}
