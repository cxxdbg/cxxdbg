// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "AppItemModel.h"
#include "CxxdbgTreeView.h"

#include <QWidget>
#include <QString>
#include <QLineEdit>
#include <QEvent>

#include "cxxdbg/dbg/auto_completion_model.hpp"


namespace cxxdbg::gui {


class AutoCompletionWidget: public QWidget {
    Q_OBJECT
public:
    enum Placement { Above, Below };

    /// Constructor
    AutoCompletionWidget(QAbstractItemModel * model, QWidget * parent);

    /// Returns number of rows in the view
    int max_visible_rows() const { return max_visible_rows_; }

    /// Sets number of rows in the view
    void set_max_visible_rows(int num_rows) { max_visible_rows_ = num_rows; }

    /// Returns max char count in a row
    int max_chars_per_line() const { return max_chars_per_line_; }

    /// Sets max char count in a row
    void set_max_chars(int max_chars) { max_chars_per_line_ =  max_chars; }

    /// Returns max items to check in model
    int max_model_items_check() const { return max_items_check_; }

    /// Sets max items to check in model
    void set_max_items_check(int num_items) { max_items_check_ = num_items; }

signals:
    /// Signals that item was doubleclicked
    void itemDoubleClicked(const QModelIndex & index);

public slots:
    /// Shows widget inscribed into parent
    void showWidget();

    /// Shows widget at specified position
    void showWidget(const QPoint& pos, Placement placement, int spacing);

    /// Hides widget
    void hideWidget();

    /// Returns selected index
    QModelIndex getSelectedIndex();

protected:
    bool eventFilter(QObject * obj, QEvent * event);

private:
    static const int max_visible_rows_def_ = 10;     ///< Max number of rows to display default value
    static const int max_chars_per_line_def_ = 25;   ///< Max number of chars per line to display default value
    QAbstractItemModel * model_;                     ///< Model
    CxxdbgTreeView * view_;                            ///< Tree view
    int max_visible_rows_;                           ///< Number of rows in view
    int max_chars_per_line_;                         ///< Max chars per line
    int max_items_check_;                            ///< Max items to check in model
};


}
