// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file LaunchOptionsWidget.cpp
/// Contains implementation of the LaunchOptionsWidget class.

#include "LaunchOptionsWidget.h"
#include "FileSelectWidget.h"
#include "FormLayout.h"
#include <QLineEdit>
#include <QMessageBox>


namespace cxxdbg::gui {


LaunchOptionsWidget::LaunchOptionsWidget(const cxxdbg::dbg::launch_options & opts,
                                         QWidget *parent):
QWidget(parent) {

    auto formLayout = new FormLayout(this);
    formLayout->setContentsMargins({});

    // working directory
    workDir_ = new FileSelectWidget(tr("Select working directory"), true);
    formLayout->addRow(tr("Working directory:"), workDir_);
    workDir_->setFileName(QString::fromStdString(opts.work_dir().string()));

    // launch arguments
    cmdArgs_ = new QLineEdit;
    formLayout->addRow(tr("Command arguments:"), cmdArgs_);
    cmdArgs_->setFocus();
    cmdArgs_->setText(QString::fromStdString(opts.launch_args_string()));
}


bool LaunchOptionsWidget::checkOptions() {
    cxxdbg::dbg::launch_options tmp_opts = opts();

    cxxdbg::dbg::launch_options::check_result res = tmp_opts.check();
    if (res == cxxdbg::dbg::launch_options::check_result_ok) {
        return true;
    }

    QString errMsg;
    QString workDir = QString::fromStdString(tmp_opts.work_dir().string());

    switch (res) {
    case cxxdbg::dbg::launch_options::check_result_work_dir_does_not_exist:
        errMsg = tr("Specified working directory '%1' does not exist. "
                    "Please specify correct directory.").arg(workDir);
        break;
    case cxxdbg::dbg::launch_options::check_result_work_dir_is_not_a_dir:
        errMsg = tr("Specified working directory '%1' is not a directory. "
                    "Please specify correct working directory.").arg(workDir);
        break;
    default:
        assert(false && "Unknown launch options check result");
        break;
    }

    QMessageBox msg(QMessageBox::Critical, tr("Error"), errMsg, QMessageBox::Ok);
    msg.exec();

    return false;
}


cxxdbg::dbg::launch_options LaunchOptionsWidget::opts() const {
    cxxdbg::dbg::launch_options tmp_opts;
    tmp_opts.set_work_dir(workDir_->fileName().toStdString());
    tmp_opts.set_launch_args(cmdArgs_->text().toStdString());
    return tmp_opts;
}


}
