// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file memory_document_widget.hpp
/// Contains definition of the memory_document_widget class.

#pragma once

#include "document_widget.hpp"


namespace cxxdbg::gui {


/// Document widget that displays memory of process being debugged
class memory_document_widget: public document_widget {
public:
    /// Constructs memory document widget with specified parent
    memory_document_widget(QWidget * parent);

    /// Sets document font
    virtual void setFont(const QFont & font) = 0;

    /// Sets tab size in document
    virtual void setTabSize(unsigned sz) = 0;

    /// Highlights search string in document, if supported
    virtual void highlightSearchString(const QString & str, bool matchCase) = 0;

    /// Removes highlighting of search string
    virtual void removeSearchStringHighlight() = 0;

    /// Jumps to the next occurrence of search string
    virtual void searchNext(const QString & str, bool matchCase) = 0;

    /// Jumps to the previous occurence of search string
    virtual void searchPrev(const QString & str, bool matchCase) = 0;
};


}
