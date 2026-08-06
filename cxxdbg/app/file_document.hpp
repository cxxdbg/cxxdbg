// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file file_document.hpp
/// Contains definition of the file_document class.

#pragma once

#include "document.hpp"
#include <filesystem>


namespace cxxdbg {


/// Represents document in application that has path in file system
class file_document: virtual public document {
public:
    using path_t = std::filesystem::path;

    /// Constructs file document with specified path
    file_document(const path_t & p): path_{p} {}

    /// Destroys file document
    virtual ~file_document() = default;

    /// Returns path of document
    auto & path() const { return path_; }

    /// Sets new document path
    void set_path(const path_t & p);

    /// Returns document name
    std::wstring name() const override {
        return path().filename().wstring();
    }

    /// Returns document name description
    std::wstring name_desc() const override;

    /// Returns true if document was changed in application after last save
    virtual bool changed() const = 0;

    /// Saves document
    virtual void save() = 0;

    /// Saves document as another file
    virtual void save_as(const path_t & p) = 0;

    /// The signal is emitted after changed flag of document changes
    CXXDBG_DEFINE_SIGNALX(changed_changed, void())

private:
    path_t path_;       ///< Document path;
};


}


