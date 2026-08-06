// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file CxxdbgProgressDialog.h
/// Contains definition of the CxxdbgProgressDialog class.

#pragma once

#include "cxxdbg/dbg/progress_dialog.hpp"
#include <QDialog>


class QProgressBar;


namespace cxxdbg::gui {


/// Implementation of the cxxdbg::app::progress_dialog class
class CxxdbgProgressDialog: public QDialog, public cxxdbg::dbg::progress_dialog {
public:
    /// Constructs progress dialog with specified title, text, range of values  and parent
    CxxdbgProgressDialog(const QString & title,
                       const QString & text,
                       int min,
                       int max,
                       QWidget * parent = nullptr);
    
    /// Constructs "busy" progress dialog with specified title, text and parent
    CxxdbgProgressDialog(const QString & title, const QString & text, QWidget * parent = nullptr):
        CxxdbgProgressDialog{title, text, 0, 0, parent} {}

    /// Destructor, destroys object
    ~CxxdbgProgressDialog() override;

    /// Shows progress dialog and blocks execution until close is called
    void show() override;

    /// Closes progress dialog
    void close() override;

protected:
    void keyPressEvent(QKeyEvent * event) override;
    void closeEvent(QCloseEvent * event) override;

    QProgressBar * progress_;       ///< Progress bar
};


}
