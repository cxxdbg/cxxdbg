// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file QSettingsStorage.cpp
/// Contains implementation of the QSettingsStorage class.

#include "QtSettingsStorage.h"
#include <QCoreApplication>


namespace cxxdbg::gui {


QtSettingsStorage::QtSettingsStorage() {
}


QtSettingsStorage::~QtSettingsStorage() {
}


bool QtSettingsStorage::read_string(const std::string & name, std::string & val) const {
    QVariant res = settings_.value(QString::fromStdString(name));
    if (res.isNull())
        return false;

    val = res.toString().toStdString();
    return true;
}


void QtSettingsStorage::write_string(const std::string & name, const std::string & val) {
    settings_.setValue(QString::fromStdString(name), QString::fromStdString(val));
}


}
