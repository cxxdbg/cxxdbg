// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file EditFunctionBreakpointDialog.h
/// Contains definition of EditFunctionBreakpointDialog class.

#pragma once

#include "AppItemModel.h"
#include "cxxdbg/app/cstring_vector_model.hpp"
#include <QDialog>


namespace cxxdbg::dbg {
    class debugger;
}


class QLineEdit;


namespace cxxdbg::gui {


/// \class EditFunctionBreakpointDialog
/// Dialog which is displayed when adding new or editing existing
/// breakpoint at function
class EditFunctionBreakpointDialog: public QDialog {
    Q_OBJECT

public:
    /// Constructor, makes dialog with specified parent
    explicit EditFunctionBreakpointDialog(cxxdbg::dbg::debugger & dbg,
                                          QWidget * parent = 0);

    /// Returns function name
    QString funcName() const;

private:
    cxxdbg::dbg::debugger & dbg_;             ///< Reference to debugger object
    std::vector<const char*> funcs_;        ///< List of functions matching string

    ///< Model for list of functions matching string
    cxxdbg::cstring_vector_model funcsModel_{&funcs_};

    AppRoItemModel qtFuncsModel_{funcsModel_};  ///< Qt model for list of functions
    QLineEdit * funcNameEdit_;                  ///< Function name line edit
};


}
