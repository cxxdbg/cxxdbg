// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file WatchListTreeView.cpp
/// Contains implementation of the WatchListTreeView class.

#include "WatchListTreeView.h"
#include "cxxdbg_connect.hpp"
#include "cxxdbg/dbg/debugger.hpp"
#include <QAction>
#include <QHeaderView>


namespace cxxdbg::gui {


WatchListTreeView::WatchListTreeView(const QString & name,
                                     cxxdbg::dbg::debugger & dbg,
                                     SortableAppRoItemModel & qtModel):
CxxdbgTreeView{name} {
    restoreState();

    setModel(&qtModel);

    // time counter column is hidden by default
    setColumnHidden(3, true);

    // say the last column to fill all space
    header()->setStretchLastSection(true);

    addSeparator();

    auto hexAct = new QAction(tr("Display hexadecimal values"), this);
    hexAct->setCheckable(true);
    hexAct->setChecked(dbg.fmt_hex());
    addAction(hexAct);
    cxxdbg_connect(hexAct, &QAction::triggered, [this, hexAct, &dbg] {
        dbg.set_fmt_hex(hexAct->isChecked());
    });

    auto ptrAddrAct = new QAction(tr("Display pointer addresses"));
    ptrAddrAct->setCheckable(true);
    ptrAddrAct->setChecked(dbg.fmt_ptr_addr());
    addAction(ptrAddrAct);
    cxxdbg_connect(ptrAddrAct, &QAction::triggered, [this, ptrAddrAct, &dbg] {
        dbg.set_fmt_ptr_addr(ptrAddrAct->isChecked());
    });

    auto recAddrAct = new QAction(tr("Display record addresses"));
    recAddrAct->setCheckable(true);
    recAddrAct->setChecked(dbg.fmt_ptr_addr());
    addAction(recAddrAct);
    cxxdbg_connect(recAddrAct, &QAction::triggered, [this, recAddrAct, &dbg] {
        dbg.set_fmt_rec_addr(recAddrAct->isChecked());
    });

    fmt_opts_changed_con_ = dbg.fmt_opts_changed().connect([this, hexAct, ptrAddrAct, recAddrAct, &dbg] {
        hexAct->setChecked(dbg.fmt_hex());
        ptrAddrAct->setChecked(dbg.fmt_ptr_addr());
        recAddrAct->setChecked(dbg.fmt_rec_addr());
    });
}


}
