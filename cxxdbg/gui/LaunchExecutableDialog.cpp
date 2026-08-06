// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file LaunchExecutableDialog.cpp
/// Contains implementation of LaunchExecutableDialog class.

#include "LaunchExecutableDialog.h"
#include "LaunchOptionsWidget.h"
#include "cxxdbg_connect.hpp"
#include "cxxdbg/dbg/debugger.hpp"
#include "cxxdbg/dbg/platform.hpp"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>


namespace cxxdbg::gui {


LaunchExecutableDialog::LaunchExecutableDialog(const cxxdbg::dbg::debugger & dbg,
                                               const cxxdbg::dbg::launch_options & opts,
                                               QWidget * parent):
QDialog(parent), dbg_{dbg} {
    setWindowTitle(tr("Starting debugging"));

    QVBoxLayout * layout = new QVBoxLayout(this);

    // options widget
    opts_ = new LaunchOptionsWidget(opts, this);
    layout->addWidget(opts_);

    // Save options check box
    saveCheckBox_ = new QCheckBox(tr("Save start options as default"), this);
    layout->addWidget(saveCheckBox_);

    // dialog buttons
    QDialogButtonBox * buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);

    cxxdbg_connect(buttons, &QDialogButtonBox::accepted, [this] { onAccepted(); });
    cxxdbg_connect(buttons, &QDialogButtonBox::rejected, [this] { reject(); });
}


cxxdbg::dbg::launch_options LaunchExecutableDialog::launch_opts() const {
    return opts_->opts();
}


bool LaunchExecutableDialog::saveOpts() const {
    return saveCheckBox_->isChecked();
}


void LaunchExecutableDialog::onAccepted() {
    // checking options
    if (dbg_.selected_platform() == dbg_.platforms().host()) {
        if (!opts_->checkOptions()) {
            return;
        }
    }

    accept();
}


}
