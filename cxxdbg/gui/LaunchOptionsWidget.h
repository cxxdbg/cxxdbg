// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file LaunchOptionsWidget.h
/// Contains definition of the LaunchOptionsWidget

#pragma once

#include "cxxdbg/dbg/launch_options.hpp"
#include <QWidget>


class QLineEdit;


namespace cxxdbg::gui {

class FileSelectWidget;


/// \class LaunchOptionsWidget
/// Contains fields for selecting launch options
class LaunchOptionsWidget: public QWidget {
public:
    /// Constructor, makes launch options widget with specified
    /// default values
    LaunchOptionsWidget(const cxxdbg::dbg::launch_options & opts,
                        QWidget * parent = nullptr);

    /// Checks launch options. Returns true if options are correct
    bool checkOptions();

    /// Returns launch options
    cxxdbg::dbg::launch_options opts() const;

private:
    FileSelectWidget * workDir_;        ///< Working directory selection widget
    QLineEdit * cmdArgs_;               ///< Command arguments
};


}
