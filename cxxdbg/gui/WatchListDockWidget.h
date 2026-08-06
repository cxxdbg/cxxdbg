// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file WatchListDockWidget.h
/// Contains definition of the WatchListDockWidget class.

#pragma once

#include "cxxdbg/app/sortable_proxy_tree_view_model.hpp"
#include "cxxdbg/dbg/custom_watch_model.hpp"

#include "AppItemModel.h"
#include "CxxdbgDockWidget.h"

#include <QAction>


namespace cxxdbg {
    class document_navigator;
}

namespace cxxdbg::dbg {
    class debugger;
}


namespace cxxdbg::gui {

class DebugUI;
class WatchListTreeView;
class CxxdbgApplication;


/// Dock widget which displays custom watch list
class WatchListDockWidget: public CxxdbgDockWidget {
public:
    /// Constructor, makes watch list dock widget with
    /// specified reference debugger and document list
    WatchListDockWidget(cxxdbg::dbg::debugger & dbg,
                        document_navigator & doc_nav,
                        DebugUI & dbgUi);

    /// Destructor, destroys widget
    virtual ~WatchListDockWidget();

    /// Saves widget state
    void saveState();
    
protected:
    virtual void keyPressEvent(QKeyEvent*);    

private:
    /// Called when user selects Add in context menu
    void onAddClicked();

    /// Called when user selected Delete in context menu
    void onDeleteClicked();

    /// Called when user selects Edit in context menu
    void onEditClicked();

    /// Returns list of indexes of selected root rows
    std::list<QModelIndex> selectedRootRows();

    /// Returns selected node
    cxxdbg::dbg::watch_list_tree_node * getSelectedNode();

    cxxdbg::dbg::custom_watch_model  model_;   ///< Watch model

    /// Sortable proxy model around watch model
    cxxdbg::sortable_proxy_tree_view_model sortableModel_;

    SortableAppItemModel qtModel_;          ///< Qt watch model
    WatchListTreeView * tree_;              ///< Pointer to watch tree view
    
    QAction * addAction_;                   ///< Add watch action
    QAction * deleteAction_;                ///< Delete watch(s) action
    QAction * editAction_;                  ///< Edit watch action

    QScopedPointer<QIcon> goto_definition_icon_;///< goto var definition icon
    QScopedPointer<QIcon> goto_value_definition_icon_;///< goto var definition icon
    QScopedPointer<QIcon> goto_typedef_icon_;   ///< goto typedef icon

    QAction * goto_definition_action_;          ///< goto definition action
    QAction * goto_value_definition_action_;    ///< goto value definition action
    QAction * goto_typedef_action_;             ///< goto typedef action

    ///< Connection to application state changed signal
    cxxdbg::scoped_signal_connection state_changed_con_;
};


}
