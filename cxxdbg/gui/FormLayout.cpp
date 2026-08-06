// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file FormLayout.cpp
/// Contains implementation of the FormLayout class.

#include "FormLayout.h"
#include <QLabel>


namespace cxxdbg::gui {


FormLayout::FormLayout(QWidget * widget):
QFormLayout{widget} {
    // use at least expanding policy on all platforms (including Macos)
    if (fieldGrowthPolicy() == QFormLayout::FieldsStayAtSizeHint) {
        setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    }
}


void FormLayout::addRowWithDesc(const QString & name, const QString & desc, QWidget * widget) {
    auto labelText = QString{"<b>%1</b><br>%2"}.arg(name, desc);
    auto label = new QLabel{labelText};
    addRow(label);
    label->setWordWrap(true);
    addRow(widget);
}


}
