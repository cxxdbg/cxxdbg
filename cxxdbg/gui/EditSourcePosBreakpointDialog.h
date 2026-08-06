// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file EditSourcePosBreakpointDialog.h
/// Contains definition of EditSourcePosBreakpointDialog class.

#pragma once

#include "AppItemModel.h"
#include <QDialog>


namespace cxxdbg {
    class ro_tree_view_model;
}

class QLineEdit;
class QComboBox;


namespace cxxdbg::gui {

class FileSelectWidget;


/// \class EditSourcePosBreakpointDialog
/// Dialog for adding new or editing existing breakpoint at source position
class EditSourcePosBreakpointDialog : public QDialog {
    Q_OBJECT

public:
    /// Constructs dialog with specified reference to source list model and parent
    explicit EditSourcePosBreakpointDialog(const cxxdbg::ro_tree_view_model * srcList = nullptr,
                                           QWidget * parent = nullptr);

    /// Returns file name
    QString fileName() const;

    /// Sets file name
    void setFileName(const QString & nm);

    /// Returns line number
    unsigned int lineNumber() const;

    /// Sets line number
    void setLineNumber(unsigned int line);

private:
    std::unique_ptr<AppRoItemModel> qtSrcListModel_;        ///< Qt model for sources list
    QComboBox * sourceNameWidget_;                          ///< Source name widget
    QLineEdit * lineNumEdit_;                               ///< Line number widget
};


}
