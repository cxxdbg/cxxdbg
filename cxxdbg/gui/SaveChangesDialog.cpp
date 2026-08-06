// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file SaveChangesDialog.cpp
/// Contains implementation of the SaveChangesDialog class.

#include "SaveChangesDialog.h"
#include "cxxdbg_connect.hpp"
#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QFileIconProvider>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>


namespace cxxdbg::gui {


SaveChangesDialog::SaveChangesDialog(const std::vector<path_t> & files, QWidget * parent):
QDialog{parent} {
    setWindowTitle(tr("Saving changes"));

    auto layout = new QVBoxLayout(this);

    auto desc = new QLabel(tr("The following files were not saved:"));
    layout->addWidget(desc);

    auto fileList = new QListWidget;
    layout->addWidget(fileList);
    QFileIconProvider iprov;
    for (auto && f : files) {
        auto path = QString::fromStdWString(f.wstring());
        auto icon = iprov.icon(QFileInfo{path});
        fileList->addItem(new QListWidgetItem{icon, path});
    }

    auto buttons = new QDialogButtonBox{QDialogButtonBox::SaveAll |
                                        QDialogButtonBox::Discard |
                                        QDialogButtonBox::Cancel};
    layout->addWidget(buttons);
    cxxdbg_connect(buttons, &QDialogButtonBox::accepted, [this] { accept(); });
    cxxdbg_connect(buttons, &QDialogButtonBox::rejected, [this] { reject(); });
    cxxdbg_connect(buttons, &QDialogButtonBox::clicked, [this, buttons](QAbstractButton * btn) {
        if (buttons->standardButton(btn) == QDialogButtonBox::Discard) {
            done(DontSave);
        }
    });
}


}
