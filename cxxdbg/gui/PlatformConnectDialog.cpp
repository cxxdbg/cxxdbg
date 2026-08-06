// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file PlatformConnectDialog.cpp
/// Contains implementation of the PlatformConnectDialog class.

#include "PlatformConnectDialog.h"
#include "PlatformWidget.h"
#include "CxxdbgProgressDialog.h"
#include "cxxdbg_connect.hpp"

#include "cxxdbg/dbg/application.hpp"
#include "cxxdbg/dbg/debugger.hpp"

#include <QDialogButtonBox>
#include <QMessageBox>
#include <QVBoxLayout>


namespace cxxdbg::gui {


PlatformConnectDialog::PlatformConnectDialog(cxxdbg::dbg::application & app, QWidget * parent):
QDialog{parent},
app_{app} {
    auto layout = new QVBoxLayout{this};

    // platform settings
    platform_ = new PlatformWidget{app_.dbg().platforms(), false, false};
    layout->addWidget(platform_);

    layout->addStretch(1);

    // dialog buttons                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
    {
        auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        layout->addWidget(buttons);
        cxxdbg_connect(buttons, &QDialogButtonBox::accepted, [this] { onOkClicked(); });
        cxxdbg_connect(buttons, &QDialogButtonBox::rejected, [this] { reject(); });
    }
}


void PlatformConnectDialog::onOkClicked() {
    auto selPlatform = platform_->selectedPlatform();
    auto url = platform_->url();

    // checking that all platform fields are correct
    if (!platform_->checkOptions()) {
        return;
    }

    // connecting to platform
    try {
        cxxdbg::dbg::platform_connection_options opts;
        opts.plat = selPlatform;
        opts.url = url.toStdString();
        opts.exec_search_paths = platform_->execSearchPaths();
        if (app_.ask_and_connect(opts)) {
            accept();
        }
    }
    catch(std::exception & err) {
        auto msg = QString{"Can't connect to platform: %1"}.arg(QString::fromStdString(err.what()));
        QMessageBox::critical(this, "Error connecting to platform", msg);
    }
}


}
