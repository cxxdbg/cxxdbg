// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file BreakpointHitCoundDialog.h
/// Contains definition of BreakpointHitCountDialog class.

#pragma once

#include <QDialog>


class QCheckBox;
class QLabel;
class QLineEdit;


namespace cxxdbg::gui {


/// \class BreakpointHitCountDialog
/// Dialog for editing breakpoint hit count property
class BreakpointHitCountDialog: public QDialog {
    Q_OBJECT

public:
    /// Constructor, makes new dialog with specified parent
    BreakpointHitCountDialog(QWidget * parent);

    /// Returns true if hit count is enabled
    bool hitCountEnabled() const;

    /// Sets whether hit count is enabled
    void setHitCountEnabled(bool val);

    /// Returns hit count value
    unsigned int hitCount() const;

    /// Sets hit count value
    void setHitCount(unsigned val);

    /// Sets current hit count
    void setCurrHitCount(unsigned val);

    /// Returns whether reset flag is set
    bool resetCurrHitCount() const;

private:
    /// Called when enable check box state changed
    void onEnableStateChanged();

    /// Called when user accepts dialog
    void onAccepted();

    QCheckBox * enableCheckBox_;        ///< Enable hit count checkbox
    QLineEdit * hitCountLineEdit_;      ///< Hit count value line edit
    QLabel * currHitCountLabel_;        ///< Current hit count label
    QCheckBox * resetCurrHitCount_;     ///< Reset current hit count checkbox
};


}
