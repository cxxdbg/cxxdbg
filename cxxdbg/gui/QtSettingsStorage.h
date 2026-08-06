// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file QtSettingsStorage
/// Contains definition of the QtSettingsStorage class.

#pragma once

#include "cxxdbg/app/settings_storage.hpp"
#include <QSettings>


namespace cxxdbg::gui {


/// \class QtSettingsStorage
/// Implementation of settings_storage via QSettings
class QtSettingsStorage: public cxxdbg::settings_storage {
public:
    /// Constructor, sets company/product name for application
    QtSettingsStorage();

    /// Destructor, destroys object
    virtual ~QtSettingsStorage();

    /// Returns reference to Qt settings object
    auto & qsettings() { return settings_; }

private:
    /// Reads string value with specified name from storage. Returns true
    /// if storage contains value.
    virtual bool read_string(const std::string & name, std::string & val) const;

    /// Writes string value with specified name to storage
    virtual void write_string(const std::string & name, const std::string & val);

    QSettings settings_;        ///< Qt settings object
};


}
