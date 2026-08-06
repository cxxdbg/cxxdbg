// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file SaveChangesDialog.h
/// Contains definition of the SaveChangesDialog class.

#pragma once

#include <QDialog>
#include <filesystem>


namespace cxxdbg::gui {


/// Dialog for saving all changed files
class SaveChangesDialog: public QDialog {
    using path_t = std::filesystem::path;

public:
    enum {
        /// Additional dialog code for "Don't save" button
        DontSave = 2
    };

    /// Constructs dialog with specified list of changed files
    SaveChangesDialog(const std::vector<path_t> & files, QWidget * parent);
};


}
