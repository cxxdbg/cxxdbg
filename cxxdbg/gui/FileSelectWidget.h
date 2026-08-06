// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <QWidget>


class QLineEdit;
class QPushButton;


namespace cxxdbg::gui {


/// \class FileSelectWidget
/// Widget for selecting file. Contains line edit and browse button
class FileSelectWidget : public QWidget {
    Q_OBJECT

public:
    explicit FileSelectWidget(const QString & dialogCaption,
                              bool dir,
                              QWidget *parent = 0);

    /// Sets file name
    void setFileName(const QString & name);

    /// Returns name of selected file
    QString fileName() const;

    /// Sets whether browse button is enabled
    void setBrowseButtonEnabled(bool value);

    /// Sets placeholder text that will be displayed when file name is empty
    void setPlaceholderText(const QString & str);

signals:
    /// The signal is emitted when value of field is changed
    void changed();

private:
    QLineEdit * fileName_;          ///< File name edit widget
    QPushButton * browseButton_;    ///< Browse button
};


}
