// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <QDockWidget>


class QVBoxLayout;
class QToolBar;


namespace cxxdbg::gui {


class CxxdbgDockWidget: public QDockWidget {
    Q_OBJECT;
public:
    /// Constructor
    CxxdbgDockWidget(const QString & title = {}, QWidget * parent = nullptr, Qt::WindowFlags flags = {});

    /// Set container's content enabled or disabled, should be used instead of QDockWidget's setEnabled()
    void setContentEnabled(bool value);

    /// Sets content widget
    void setWidget(QWidget * content);

    /// Returns pointer to toolbar
    QToolBar * toolBar();

private:
    QToolBar * toolBar_ = nullptr;                  ///< Toolbar
    QVBoxLayout * layout_ = nullptr;                ///< Widget layout
};


}
