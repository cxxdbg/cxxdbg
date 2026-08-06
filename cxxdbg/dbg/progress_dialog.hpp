// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file progress_dialog.hpp
/// Contains definition of the progress_dialog class.

#pragma once

namespace cxxdbg::dbg {


/// \class progress_dialog
/// Abstract progress dialog which is used by application class
/// to display progress
class progress_dialog {
public:
    /// Destructor, destroys object
    virtual ~progress_dialog() {}

    /// Shows progress dialog and blocks execution until close is called
    virtual void show() = 0;

    /// Closes progress dialog
    virtual void close() = 0;
};


}


