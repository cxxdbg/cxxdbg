// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <QDialog>


class QLineEdit;


namespace cxxdbg::gui {


/// \class LineNumberDialog
/// Dialog for selecting line number where to jump
class LineNumberDialog: public QDialog {
    Q_OBJECT

public:
    /// Constructor, makes line number selectiondialog with specified parent
    LineNumberDialog(unsigned int firstLine, unsigned int lastLine, QWidget * parent = 0);

    /// Returns line number
    unsigned long lineNumber() const { return selectedLine_; }

private:
    /// Called when user clicks Ok buttom
    void onOkClicked();
    
    unsigned int selectedLine_;
    unsigned lastLine_;
    
    QLineEdit * lineNumberEdit_;       ///< line number editor field
};


}
