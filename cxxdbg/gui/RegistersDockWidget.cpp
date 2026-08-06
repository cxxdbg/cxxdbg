// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file RegistersDockWidget.cpp
/// Contains implementation of RegistersDockWidget class.

#include "RegistersDockWidget.h"
#include "WatchListTreeView.h"
#include "WatchListColorConverter.h"
#include "Utils.h"
#include "cxxdbg_connect.hpp"

#include "cxxdbg/app/tree_view_model.hpp"
#include "cxxdbg/dbg/debugger.hpp"
#include "cxxdbg/dbg/watch_list.hpp"
#include "cxxdbg/dbg/watch_model.hpp"

#include <QToolBar>
#include <QCoreApplication>
#include <QtWidgets/QtWidgets>
#include <QAbstractItemView>


namespace cxxdbg::gui {


RegistersDockWidget::RegistersDockWidget(cxxdbg::dbg::debugger & dbg):
CxxdbgDockWidget{tr("Registers")},
regModel_{dbg.registers()},
sortableModel_{regModel_},
regQtModel_{sortableModel_}
{

    setObjectName("locals");

    regQtModel_.setColorConverter(WatchListColorConverter::createInstance());

    // creating watch tree
    tree_ = new WatchListTreeView{"ui/locals/tree", dbg, regQtModel_};
    tree_->setFrameStyle(QFrame::NoFrame);
    setWidget(tree_);
}


RegistersDockWidget::~RegistersDockWidget() {
}


void RegistersDockWidget::saveState() {
    tree_->saveState();
}


}
