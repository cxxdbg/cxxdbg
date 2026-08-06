// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "LoadExecutableDialog.h"
#include "FileSelectWidget.h"
#include "LaunchOptionsWidget.h"
#include "PlatformWidget.h"
#include "CxxdbgApplication.h"
#include "cxxdbg_connect.hpp"

#include "cxxdbg/dbg/debugger.hpp"

#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>


namespace cxxdbg::gui {


LoadExecutableDialog::LoadExecutableDialog(cxxdbg::dbg::application & app, QWidget *parent):
QDialog(parent),
app_{app} {

    setWindowTitle(tr("Loading executable"));
    QVBoxLayout * layout = new QVBoxLayout(this);

    // executable file and target name
    auto desc = "Executable file or target name on destination platform.";
    targetLabel_ = new QLabel{desc};
    targetLabel_->setWordWrap(true);
    layout->addWidget(targetLabel_);

    // target name or exe file
    fileName_ = new FileSelectWidget(tr("Select executable file for loading"), false, this);
    layout->addWidget(fileName_);

    // launch options
    QGroupBox * launchGrp = new QGroupBox(tr("Default start options"), this);
    layout->addWidget(launchGrp);
    QVBoxLayout * launchGrpLayout = new QVBoxLayout(launchGrp);
    launchOpts_ = new LaunchOptionsWidget(app_.def_launch_opts(), this);
    launchGrpLayout->addWidget(launchOpts_);

    // platform options
    QGroupBox * platformGrp = new QGroupBox{tr("Platform options")};
    layout->addWidget(platformGrp);
    QVBoxLayout * platformGrpLayout = new QVBoxLayout{platformGrp};
    platformOpts_ = new PlatformWidget{app_.dbg().platforms(), true, true};
    platformGrpLayout->addWidget(platformOpts_);
    cxxdbg_connect(platformOpts_, &PlatformWidget::selectedPlatformChanged, [this] { onSelectedPlatformChanged(); });

    // dialog buttons
    {
        QDialogButtonBox * buttons =
                new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        layout->addWidget(buttons);

        cxxdbg_connect(buttons, &QDialogButtonBox::accepted, [this] { onOkClicked(); });
        cxxdbg_connect(buttons, &QDialogButtonBox::rejected, [this] { reject(); });
    }


    setFixedHeight(sizeHint().height());

    onSelectedPlatformChanged();
}


QString LoadExecutableDialog::fileName() const {
    return fileName_->fileName();
}


cxxdbg::dbg::launch_options LoadExecutableDialog::defLaunchOpts() const {
    return launchOpts_->opts();
}


cxxdbg::dbg::platform_connection_options LoadExecutableDialog::platformOpts() const {
    return platformOpts_->platformOpts();
}


void LoadExecutableDialog::onOkClicked() {
    // checking platform options
    if (!platformOpts_->checkOptions()) {
        return;
    }

    auto p = platformOpts_->selectedPlatform();
    if (p == nullptr) {
        p = app_.dbg().connected_platform();
    }

    auto targName = fileName();

    // checking that file name is not empty
    if (fileName().isEmpty()) {
        QMessageBox msg(QMessageBox::Critical, tr("Error"),
                        tr("Executable file or target name is not specified. "
                        "Please specify correct file or target name."));
        msg.exec();
        return;
    }

    // check file exists for host platform

    if (p == app_.dbg().platforms().host()) {
        QFile file(fileName());
        if (!file.exists()) {
            QMessageBox msg(QMessageBox::Critical, tr("Error"),
                            tr("Specified file '%1' does not exist. "
                            "Please specify correct file name.").arg(fileName()));
            msg.exec();
            return;
        }
    }

    // checking launch options
    if (p == app_.dbg().platforms().host()) {
        if (!launchOpts_->checkOptions()) {
            return;
        }
    }

    // loading target
    if (!app_.ask_and_load_target(targName.toStdString(), defLaunchOpts(), platformOpts())) {
        return;
    }

    // closing dialog if loading was completed
    accept();
}


void LoadExecutableDialog::onSelectedPlatformChanged() {
    auto p = platformOpts_->selectedPlatform();
    if (p == nullptr) {
        // current platform
        p = app_.dbg().connected_platform();
    }

    assert(p != nullptr && "invalid platform");
    if (p->target_field_name().empty()) {
        fileName_->setBrowseButtonEnabled(true);
    } else {
        fileName_->setBrowseButtonEnabled(false);
    }
}


}
