// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file FileSystemDockWidget.h
/// Contains definition of FileSystemDockWidget class.

#pragma once

#include "CxxdbgDockWidget.h"


class QToolBar;


namespace cxxdbg::gui {


/// \class FileSystemDockWidget
/// Dock widget which displays file system tree for opening source files
class FileSystemDockWidget : public CxxdbgDockWidget {
    Q_OBJECT

public:
    /// Constructor, makes new file system dock widget
    explicit FileSystemDockWidget(QWidget * parent);


signals:
    /// Emitted when user selects open file
    void openSelected(const QString &);


private:
    /// Updates menu status
    void updateMenuStatus();

    QAction * openFileAction_;      ///< Pointer to
};


}
