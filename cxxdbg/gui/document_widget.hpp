// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file document_widget.h
/// Contains definition of the document_widget class.

#pragma once

#include <QWidget>


namespace cxxdbg {
    class document;
}


namespace cxxdbg::gui {


/// Abstract widgets that displays document in the list of documents.
class document_widget: public QFrame {
public:
    /// Constructs document widget with specified parent widget
    document_widget(QWidget * parent): QFrame{parent} {}

    /// Sets document font
    virtual void set_font(const QFont & font) = 0;

    /// Sets tab size in document
    virtual void set_tab_size(unsigned sz) = 0;

    /// Highlights search string in document, if supported
    virtual void highlight_search_string(const QString & str, bool matchCase) = 0;

    /// Removes highlighting of search string
    virtual void remove_search_string_highlight() = 0;

    /// Jumps to the next occurrence of search string
    virtual void search_next(const QString & str, bool match_case) = 0;

    /// Jumps to the previous occurence of search string
    virtual void search_prev(const QString & str, bool match_case) = 0;

    /// Returns pointer to document
    virtual document * doc() = 0;
};


/// Abstract factory for creating widgets for documents
class document_widget_factory {
public:
    /// Destroys factory
    virtual ~document_widget_factory() = default;

    /// Creates document widget for specified document
    virtual std::unique_ptr<document_widget> create_document_widget(cxxdbg::document & doc) = 0;
};


}
