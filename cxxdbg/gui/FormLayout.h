// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file FormLayout
/// Contins definition of the FormLayout class.

#pragma once

#include <QFormLayout>


namespace cxxdbg::gui {


/// Form layout derived from QFormLayout with default setup for some values.
/// Should be used instead of QFormLayout in all code.
class FormLayout: public QFormLayout {
public:
    /// Constructs form layout for specified widget
    FormLayout(QWidget * widget = nullptr);

    /// Adds row with description
    void addRowWithDesc(const QString & name, const QString & desc, QWidget * widget);
};


}
