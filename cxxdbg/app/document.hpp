// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file document.hpp
/// Contains implementation of the document class.

#pragma once

#include "cxxdbg/app/signals.hpp"
#include <string>


namespace cxxdbg {


/// Base class for all documents in application
class document {
public:
    /// Virtual destructor
    virtual ~document() = default;

    /// Returns document name
    virtual std::wstring name() const = 0;

    /// Returns document name description that should be used to display document
    /// name and cotnains additional attributes like changed flags
    virtual std::wstring name_desc() const { return name(); }


    /// Returns true if can perform undo in document
    virtual bool can_undo() const = 0;

    /// Returns true if can perform redo in document
    virtual bool can_redo() const = 0;

    /// Returns true if can perform cut in document
    virtual bool can_cut() const = 0;

    /// Returns true if can perform copy in document
    virtual bool can_copy() const = 0;

    /// Returns true if can perform paste in document
    virtual bool can_paste() const = 0;

    /// Returns true if can perform delete in document
    virtual bool can_del() const = 0;

    /// Returns true if can perform select all in document
    virtual bool can_select_all() const = 0;

    /// Returns true if can perform go to line in document
    virtual bool can_go_to_line() const = 0;

    /// Performs undo in document
    virtual void undo() = 0;

    /// Performs redo in document
    virtual void redo() = 0;

    /// Performs cut in document. Returns string that should be inserted into clipboard
    virtual std::wstring cut() = 0;

    /// Performs copy in document. Returns string that should be inserted into clipboard
    virtual std::wstring copy() = 0;

    /// Performs paste in document
    virtual void paste(const std::wstring & text) = 0;

    /// Performs delete in document.
    virtual void del() = 0;

    /// Performs select all in document
    virtual void select_all() = 0;

    /// Performs go to line in document
    virtual void go_to_line(size_t l) = 0;

    /// Returns number of lines in document for go to line function
    virtual size_t lines_count() const = 0;

    /// The signal is emitted when document name changes
    CXXDBG_DEFINE_SIGNALX(name_changed, void())

    /// The signal is emitted when one or more document eidition can_* options changed
    CXXDBG_DEFINE_SIGNALX(edit_options_changed, void())
};


}


