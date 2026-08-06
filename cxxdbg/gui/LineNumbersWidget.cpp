// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "LineNumbersWidget.h"
#include "text_document_widget.hpp"
#include <QPainter>
#include <QTextBlock>


namespace cxxdbg::gui {


LineNumbersWidget::LineNumbersWidget(text_document_widget_base * srcCode, QWidget * parent):
QWidget(parent),
sourceCodeWidget_(srcCode) {
}


QSize LineNumbersWidget::minimumSizeHint() const {
    return sizeHint();
}


QSize LineNumbersWidget::sizeHint() const {
    return QSize(sourceCodeWidget_->linesNumbersWidth(), 0);
}


void LineNumbersWidget::paintEvent(QPaintEvent * event) {
    sourceCodeWidget_->handleLineNumbersPaint(event);
}


void LineNumbersWidget::mouseMoveEvent(QMouseEvent * event) {
    sourceCodeWidget_->handleLineNumbersMouseMove(event);
}


void LineNumbersWidget::mouseReleaseEvent(QMouseEvent * event) {
    sourceCodeWidget_->handleLineNumbersClick(event);
}


}
