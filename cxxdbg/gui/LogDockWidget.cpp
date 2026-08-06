// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file LogDockWidget.cpp
/// Contains implementation of the LogDockWidget class.

#include "LogDockWidget.h"
#include <QPlainTextEdit>
#include <QVBoxLayout>


namespace cxxdbg::gui {


LogDockWidget::LogDockWidget(cxxdbg::application_log & l):
CxxdbgDockWidget{tr("Debug Log")},
log_{l} {
    setObjectName("logdockwidget");

    logEdit_ = new QPlainTextEdit;
    setWidget(logEdit_);
    logEdit_->setFrameStyle(QFrame::NoFrame);
}


void LogDockWidget::showEvent(QShowEvent * event) {
    // displaying old log records
    for (auto && rec : log_.old_records()) {
        logEdit_->appendPlainText(QString::fromStdString(rec));
    }

    // listening for new log records
    con_ = log_.record_received().connect([this](auto && msg) {
        logEdit_->appendPlainText(QString::fromStdString(msg));
    });
}


void LogDockWidget::hideEvent(QHideEvent * event) {
    con_.disconnect();
}


}
