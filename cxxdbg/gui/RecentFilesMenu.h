// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file RecentFilesMenu.h
/// Contains definition of the RecentFilesMenu class.

#pragma once

#include "cxxdbg/app/recent_file_list.hpp"
#include <QMenu>


namespace cxxdbg::gui {


/// Menu that displays items from recent_file_list objects and updates itself
/// when recent_file_list object changes
class RecentFilesMenu: public QMenu {
    Q_OBJECT

public:
    /// Constructs menu with reference to file list object
    RecentFilesMenu(cxxdbg::recent_file_list & flist, const QString & title);

signals:
    /// The signal is emitted when user selects file
    void fileSelected(const QString & path);

private:
    /// Updates menu items
    void updateItems();

    cxxdbg::recent_file_list & files_;                ///< Reference to file list object
    cxxdbg::util::scoped_signal_connection con_;      ///< Connection to file list changed signal

    QAction * clearAction_ = nullptr;
};


}
