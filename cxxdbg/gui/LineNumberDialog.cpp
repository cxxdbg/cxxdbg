// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "LineNumberDialog.h"
#include "FormLayout.h"
#include "cxxdbg_connect.hpp"

#include <QDialogButtonBox>
#include <QIntValidator>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>
#include <cassert>


namespace cxxdbg::gui {


LineNumberDialog::LineNumberDialog(unsigned int firstLine, unsigned int lastLine, QWidget * parent):
QDialog(parent), selectedLine_(firstLine), lastLine_{lastLine} {

    setWindowTitle(tr("Select line number"));
    QVBoxLayout * layout = new QVBoxLayout(this);

    // dialog form
    {
        auto formLayout = new FormLayout;
        layout->addLayout(formLayout);

        // line number row
        lineNumberEdit_ = new QLineEdit;
        formLayout->addRow(
            tr("Select line number ") + QString::number(firstLine) + ".." + QString::number(lastLine), 
                lineNumberEdit_
        );
        
        lineNumberEdit_->setValidator(new QIntValidator(firstLine, lastLine));
    }

    // dialog buttons
    {
        QDialogButtonBox * buttons =
                new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        
        layout->addWidget(buttons);

        cxxdbg_connect(buttons, &QDialogButtonBox::accepted, [this] { onOkClicked(); });
        cxxdbg_connect(buttons, &QDialogButtonBox::rejected, [this] { reject(); });
    }

    setFixedHeight(sizeHint().height());
}


void LineNumberDialog::onOkClicked() {
    selectedLine_ = lineNumberEdit_->text().toInt();
    if (selectedLine_ < 1 || selectedLine_ > lastLine_) {
        QMessageBox::critical(this,
                              tr("Invlid line number"),
                              tr("Line number is invalid for current document"));
        return;
    }

    accept();
}


}
