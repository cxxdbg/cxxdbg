// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file SettingsDialog.h
/// Contains definition of the SettingsDialog class.

#pragma once

#include <QDialog>


class QDialogButtonBox;
class QStackedWidget;
class QTreeWidget;
class QTreeWidgetItem;


namespace cxxdbg::gui {


/// Settings dialog
class SettingsDialog: public QDialog {
    Q_OBJECT

public:
    /// Constructs settings dialog with specified pointer to parent
    SettingsDialog(bool displayRestore, QWidget * parent = nullptr);

signals:
    /// Emitted when user clicks apply button
    void applyClicked();

    /// Emitted when user clicks restore defaults button
    void restoreDefaultsClicked();

protected:
    /// Adds settings page. Returns tree widget item for page added.
    QTreeWidgetItem * addPage(const QString & name, QWidget * page, QTreeWidgetItem * parent = nullptr);

    /// Sets enabled state of apply button
    void setApplyEnabled(bool enabled);

private:
    QTreeWidget * menu_;                ///< Menu tree
    QStackedWidget * stack_;            ///< Stack widget for settings pages
    QDialogButtonBox * buttons_;        ///< Dialog buttons widget
};


}
