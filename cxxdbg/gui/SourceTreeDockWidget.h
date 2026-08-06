// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file SourceTreeDockWidget
/// Contains definition of SourceTreeDockWidget class.

#pragma once

#include "cxxdbg/app/signals.hpp"
#include "cxxdbg/dbg/source_tree.hpp"

#include <QIcon>

#include "AppItemModel.h"
#include "CxxdbgDockWidget.h"


class QAction;
class QTreeWidget;
class QTreeView;
class QTreeWidgetItem;
class QStyle;

namespace cxxdbg::dbg {
    class composite_source_tree_model;
}


namespace cxxdbg::gui {

class IconManager;
class CxxdbgTreeView;
class CxxdbgApplication;


/// \class SourceTreeDockWidget
/// Dock widget which displays source tree from loaded executable
class SourceTreeDockWidget : public CxxdbgDockWidget {
Q_OBJECT

public:
    /// Constructor, makes widget with specified reference to application object
    explicit SourceTreeDockWidget(CxxdbgApplication & app);

    ~SourceTreeDockWidget();

private:
    /// called when file selection changed
    void onSelectionChanged(const QModelIndex & current, const QModelIndex & previous);

    /// Called when source list changed
    void onSourcesChanged();

    const cxxdbg::dbg::source_tree_item * get_tree_item(const QModelIndex & index);

    /// Returns true if tree item is file
    bool isExistingFileItem(const QModelIndex &index);

    CxxdbgApplication & app_;                             ///< Reference to application object
    AppRoItemModel::ImageConverterSP image_converter_;  ///< image converter

    ///< Tree view model containing source tree
    const cxxdbg::dbg::composite_source_tree_model & model_;

    AppRoItemModel qtModel_;                ///< qt model
    CxxdbgTreeView * tree_;                   ///< Source tree view
    QAction * openSourceAction_;            ///< Open source button action

    /// Connection to sources changed signal
    cxxdbg::scoped_signal_connection sources_changed_con_;

    /// Connection to target changed signal in application
    cxxdbg::scoped_signal_connection target_changed_con_;
};


}
