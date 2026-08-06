// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "AppItemModel.h"

#include <memory>
#include <iostream>

#include <QColor>


namespace cxxdbg::gui {


/// class WatchListColorConverter is in charge of correspondence between color index and actual QColor
/// defines color for marked text
class WatchListColorConverter: public AppRoItemModel::ColorConverter {
public:
    typedef AppRoItemModel::ColorConverter BaseType;

    /// Destructor, destroys object
    virtual ~WatchListColorConverter() {}

    /// Converts color index to color
    QColor get(size_t idx) const override {
        switch (idx) {
            case 0:
                return QColor(0,0,0);
            case 1:
                return QColor(166, 8, 0);
            default:
                assert(false && "unknown color index, failed to convert");
        }

        return {};
    }

    static AppRoItemModel::ColorConverterSP createInstance() {
        return std::make_shared<WatchListColorConverter>();
    }
};


}
