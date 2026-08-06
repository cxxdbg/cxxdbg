// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file text_document.hpp
/// Contains definition of the text_document class.

#pragma once

#include "document.hpp"
#include "tmvc/position.hpp"


namespace cxxdbg {


/// Represents document in application which is displayed as text
class text_document: virtual public document {
public:
    /// Returns true if can perform undo in document
    bool can_undo() const override { return false; }

    /// Returns true if can perform redo in document
    bool can_redo() const override { return false; }

    /// Returns true if can perform cut in document
    bool can_cut() const override { return false; }

    /// Returns true if can perform paste in document
    bool can_paste() const override { return false; }

    /// Returns true if can perform delete in document
    bool can_del() const override { return false; }

    /// Returns true if can perform select all in document
    bool can_select_all() const override { return true; }

    /// Returns true if can perform go to line in document
    bool can_go_to_line() const override { return true; }

    /// Returns string at specified line
    virtual std::wstring line_str(size_t l) const = 0;

    /// Returns current position in text document
    virtual tmvc::position current_pos() const = 0;

    /// Sets tab size for editable document
    virtual void set_tab_size(unsigned val) = 0;

    /// Sets whether tabs should be expanded to spaces in document
    virtual void set_expand_tabs(bool val) = 0;
};


}


