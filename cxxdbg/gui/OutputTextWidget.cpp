// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file OutputTextWidget.cpp
/// Contains implementation of the OutputTextWidget class.

#include "OutputTextWidget.h"
#include <QScrollBar>


namespace cxxdbg::gui {


void OutputTextWidget::addText(const QString & str) {
    addText(str, palette().color(QPalette::WindowText));
}


void OutputTextWidget::addText(const QString & str, const QColor & col) {
    // detect if terminal is scrolled to the end
    auto sb = verticalScrollBar();
    bool atEnd = sb->sliderPosition() == sb->maximum();

    auto cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    QTextCharFormat fmt;
    fmt.setForeground(col);
    cursor.setCharFormat(fmt);
    cursor.insertText(str);

    // scrolling to the end if we was in the end
    if (atEnd) {
        sb->setSliderPosition(sb->maximum());
    }
}


}
