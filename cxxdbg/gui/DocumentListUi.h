// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file DocumentListUi.h
/// Contains definition of the DocumentListUi class.

#pragma once

#include "cxxdbg/app/document_list.hpp"


class QWidget;


namespace cxxdbg::gui {


/// Delegate for UI actions in document list. Subclasses must implement
/// getMainWindow method that returns pointer to main window which will
/// be parent for all message dialogs
class DocumentListUi: public cxxdbg::document_list_ui {
public:
    /// Asks user for opening big file with specified path and size
    bool ask_open_big_file(const std::filesystem::path & p, size_t sz) const override;
    
    /// Asks user for saving modified file. Returns false if user cancelled action.
    bool ask_save_files(const std::list<const cxxdbg::file_document*> & files, bool & save) const override;

private:
    /// Returns pointer to main window
    virtual QWidget * getMainWindow() const = 0;
};


}
