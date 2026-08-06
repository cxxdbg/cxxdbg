// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file OutputTextWidget.h
/// Contains definition of the OutputTextWidget class.

#pragma once

#include <QPlainTextEdit>


namespace cxxdbg::gui {


/// Widget for displaying log-style output (build, log, exe output)
class OutputTextWidget: public QPlainTextEdit {
public:
    /// Constructs output text widget with specified parent
    OutputTextWidget(QWidget * parent = nullptr): QPlainTextEdit(parent) {}

    /// Appends text with default text color to output
    void addText(const QString & str);

    /// Appends text with specified color to output
    void addText(const QString & str, const QColor & col);
};


}
