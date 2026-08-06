// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <QWidget>


namespace cxxdbg::gui {


class text_document_widget_base;


/// \class LineNumbersWidget
/// Widget for diplaying line numbers in source code widget
class LineNumbersWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LineNumbersWidget(text_document_widget_base * srcCode, QWidget * parent = 0);

    /// Returns minimum size for widget
    virtual QSize minimumSizeHint() const;

    /// Returns preferred size for widget
    virtual QSize sizeHint() const;

protected:
    /// Paint event handler
    virtual void paintEvent(QPaintEvent * event);

    /// Mouse move event handler
    virtual void mouseMoveEvent(QMouseEvent * event);

    /// Mouse click event handler
    virtual void mouseReleaseEvent(QMouseEvent * event);

private:
    text_document_widget_base * sourceCodeWidget_;      ///< Pointer to source code widget
};


}
