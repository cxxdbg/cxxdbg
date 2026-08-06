// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "cxxdbg/dbg/debug_settings.hpp"
#include <QWidget>


class QCheckBox;


namespace cxxdbg::gui {


/// \class DebugSettingsWidget
/// Displays debug settings
class DebugSettingsWidget
        : public QWidget
{
    Q_OBJECT;

    public:
        DebugSettingsWidget(const cxxdbg::dbg::debug_settings & sett,
                            QWidget *                         parent = nullptr);

        /// Returns terminal settings entered by user
        const cxxdbg::dbg::debug_settings& debugSettings() const;

    signals:
        void changed();

    private:
        cxxdbg::dbg::debug_settings debug_settings_;  ///< debug settings
        
        QCheckBox * avoidNodebugCheck_;    ///< "Avoid functions without debug information" check box
};


}
