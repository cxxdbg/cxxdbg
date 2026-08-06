// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file RegistersDockWidget.h
/// Contains definition of RegistersDockWidget class.

#pragma once

#include "AppItemModel.h"
#include "CxxdbgDockWidget.h"

#include "cxxdbg/app/sortable_proxy_tree_view_model.hpp"
#include "cxxdbg/dbg/watch_model.hpp"


#include <map>
#include <QIcon>


namespace cxxdbg {
    class document_list;
}

namespace cxxdbg::dbg {
    class debugger;
    class watch_list_tree_node;
    class watch_model;
}


namespace cxxdbg::gui {

class WatchListTreeView;


/// Dock widget for displaying locals
class RegistersDockWidget: public CxxdbgDockWidget {
public:
    /// Constructor, makes widget with specified reference to debugger
    RegistersDockWidget(cxxdbg::dbg::debugger & dbg);

    /// Destructor, destroys object
    virtual ~RegistersDockWidget();

    /// Saves state of dock widget
    void saveState();

private:
    /// Returns selected node
    cxxdbg::dbg::watch_list_tree_node * getSelectedNode();

    cxxdbg::dbg::watch_model regModel_;           ///< App model for locals watch list

    /// Sortable proxy model around watch model
    cxxdbg::sortable_proxy_ro_tree_view_model sortableModel_;

    SortableAppRoItemModel regQtModel_;         ///< Qt model for registers watch list
    WatchListTreeView * tree_;                  ///< Watch tree
};


}
