// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file FontSelectWidget.h
/// Contains definition of the FontSelectWidget class.

#pragma once

#include <QWidget>


class QLineEdit;
class QPushButton;


namespace cxxdbg::gui {


/// \class FontSelectWidget
/// Widget for selecting font
class FontSelectWidget: public QWidget {
    Q_OBJECT

public:
    FontSelectWidget(const QFont & font,
                     const QFont & defaultFont,
                     QWidget * parent = nullptr);

    /// Returns selected font
    QFont selectedFont() const;

    /// Sets selected font
    void selectFont(const QFont & font);

signals:
    /// Called after user changed font
    void changed();

private:
    QLineEdit * preview_;       ///< Font preview
    QPushButton * resetButton_; ///< Reset button
    QFont selectedFont_;        ///< Selected font
    QFont defaultFont_;         ///< Default font
};


}
