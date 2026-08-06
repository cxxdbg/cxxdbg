// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file LaunchExecutableDialog.h
/// Contains definition of LaunchExecutableDialog class.

#pragma once

#include "cxxdbg/dbg/launch_options.hpp"
#include <QDialog>


class QCheckBox;

namespace cxxdbg::dbg {
    class debugger;
}


namespace cxxdbg::gui {

class LaunchOptionsWidget;


/// \class LaunchExecutableDialog
/// Dialog which is displayed when user selects "Start debugging",
/// contains executable launching options.
class LaunchExecutableDialog: public QDialog {
    Q_OBJECT

public:
    /// Constructor, makes new dialog
    LaunchExecutableDialog(const cxxdbg::dbg::debugger & dbg,
                           const cxxdbg::dbg::launch_options & opts,
                           QWidget * parent = NULL);

    /// Returns launch options
    cxxdbg::dbg::launch_options launch_opts() const;

    /// Returns true if options should be saved as default
    bool saveOpts() const;

private:
    /// Called when user accepts dialog
    void onAccepted();

    const cxxdbg::dbg::debugger & dbg_;
    LaunchOptionsWidget * opts_;
    QCheckBox * saveCheckBox_;
};


}
