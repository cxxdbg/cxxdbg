// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <QDialog>
#include <QPixmap>


class QPlainTextEdit;


namespace cxxdbg::gui {


class AboutDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AboutDialog(const QString & productName,
                         QWidget *parent = 0);

signals:

public slots:

private:
    QPixmap logoPixmap_;            ///< Logo pixmap
};


}
