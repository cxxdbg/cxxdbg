// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file EditAddressBreakpointDialog.h
/// Contains definition of EditAddressBreakpointDialog class.

#pragma once

#include <QDialog>


class QLineEdit;


namespace cxxdbg::gui {


/// \class EditAddressBreakpointDialog
/// Represents dialog for editing breakpoint at specified address.
class EditAddressBreakpointDialog: public QDialog {
    Q_OBJECT

public:
    /// Constructor, makes dialog with specified parent
    explicit EditAddressBreakpointDialog(QWidget * parent = 0);

    /// Returns breakpoint address
    std::uint64_t address() const;

private:
    QLineEdit * addrEdit_;          ///< Address line edit
};


}
