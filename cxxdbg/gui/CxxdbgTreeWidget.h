// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file CxxdbgTreeWidget.h
/// Contains definition of CxxdbgTreeWidget class.

#pragma once

#include <QTreeWidget>


namespace cxxdbg::gui {


/// \class CxxdbgTreeWidget
/// Tree widget with extended functionality like selecting visible columns
/// and saving/restoring state
class CxxdbgTreeWidget: public QTreeWidget {
public:
    /// Constructor, makes tree widget with specified name and parent
    CxxdbgTreeWidget(const QString & nm, QWidget * parent = NULL);

    /// Adds header context menu which contains selectable items for
    /// each column except of the first column
    void addColumnSelectContextMenu();

    /// Saves state to settings
    void saveState();

    /// Restores state from settings
    void restoreState();

    /// Copies selected items to clipboard
    void copySelectedToClipboard();

    /// Adds separator to context menu
    void addSeparator();

    QString name_;          ///< Widget name, is used for saving/restoring state
};


}
