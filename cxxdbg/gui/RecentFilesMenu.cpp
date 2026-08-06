// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file RecentFilesMenu.cpp
/// Contains implementation of the RecentFilesMenu class.

#include "RecentFilesMenu.h"
#include "cxxdbg_connect.hpp"


namespace cxxdbg::gui {


RecentFilesMenu::RecentFilesMenu(cxxdbg::recent_file_list & flist, const QString & title):
QMenu{title},
files_{flist} {
    con_ = files_.changed().connect([this] { updateItems(); });

    cxxdbg_connect(this, &QMenu::triggered, [this](QAction * action) {
        if (action == clearAction_) {
            files_.clear();
            return;
        }

        auto fpath = action->data().toString();
        emit fileSelected(fpath);
    });

    updateItems();
}


void RecentFilesMenu::updateItems() {
    clear();

    for (auto && f : files_.files()) {
        auto fpath = QString::fromStdWString(f.wstring());
        QAction * action = addAction(fpath);
        action->setData(fpath);
    }

    addSeparator();

    clearAction_ = addAction(tr("Clear history"));
    clearAction_->setEnabled(!files_.files().empty());
}


}
