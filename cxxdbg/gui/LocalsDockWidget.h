// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file LocalsDockWidget.h
/// Contains definition of LocalsDockWidget class.

#pragma once

#include "AppItemModel.h"
#include "CxxdbgDockWidget.h"

#include "cxxdbg/app/sortable_proxy_tree_view_model.hpp"
#include "cxxdbg/dbg/watch_model.hpp"


#include <map>
#include <QIcon>


namespace cxxdbg {
    class document_navigator;
}

namespace cxxdbg::dbg {
    class debugger;
    class watch_list_tree_node;
    class watch_model;
}


namespace cxxdbg::gui {

class WatchListTreeView;


/// Dock widget for displaying locals
class LocalsDockWidget: public CxxdbgDockWidget {
public:
    /// Constructor, makes widget with specified reference to debugger and document navigator
    LocalsDockWidget(cxxdbg::dbg::debugger & dbg, document_navigator & doc_nav);

    /// Destructor, destroys object
    virtual ~LocalsDockWidget();

    /// Saves state of dock widget
    void saveState();

private:
    /// Returns selected node
    cxxdbg::dbg::watch_list_tree_node * getSelectedNode();

    cxxdbg::dbg::watch_model locModel_;           ///< App model for locals watch list

    /// Sortable proxy model around watch model
    cxxdbg::sortable_proxy_ro_tree_view_model sortableModel_;

    SortableAppRoItemModel locQtModel_;         ///< Qt model for locals watch list
    WatchListTreeView * tree_;                  ///< Watch tree

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
