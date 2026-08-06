// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file cxxdbg_connect.cpp
/// Contains implementation of the cxxdbg_connect related functions.

#include "cxxdbg_connect.hpp"
#include <QMessageBox>


namespace cxxdbg::gui {


void cxxdbg_connect_handle_error(const std::exception & err) {
    QMessageBox msg(QMessageBox::Critical,
                    QObject::tr("Error"),
                    QObject::tr("Error: %1").arg(QString::fromStdString(err.what())),
                    QMessageBox::Ok);
    msg.exec();
}


void cxxdbg_connect_handle_unknown_error() {
    QMessageBox msg(QMessageBox::Critical,
                    QObject::tr("Error"),
                    QObject::tr("Unknown Error."),
                    QMessageBox::Ok);
    msg.exec();
}


}
