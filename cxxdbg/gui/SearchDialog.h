// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file SearchDialog.h
/// Contains definition of the SearchDialog class.

#pragma once

#include <QDialog>

class QCheckBox;
class QLineEdit;
class QPushButton;


namespace cxxdbg::gui {

class document_list_widget;
class MainWindow;


/// \class SearchDialog
/// Search dialog
class SearchDialog: public QDialog {
public:
    /// Constructor, makes search dialog
    SearchDialog(document_list_widget * docsWidget, QWidget * parent = nullptr);

protected:
    /// Processes show event. Sets focus to search string and enables highlight
    /// in main window
    virtual void showEvent(QShowEvent * event);

private:
    /// Updates search buttons state
    void updateSearchButtons();

    /// Called when search conditions changed
    void onSearchCondChanged();

    document_list_widget * docsWidget_; ///< Pointer to document list widget
    QLineEdit * searchStrEdit_;         ///< Search string line edit
    QCheckBox * matchCaseCheckBox_;     ///< Match case check box
    QPushButton * findNextButton_;      ///< Find next button
    QPushButton * findPrevButton_;      ///< Find previous button
};


}
