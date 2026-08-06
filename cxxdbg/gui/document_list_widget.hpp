// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file document_list_widget.hpp
/// Contains definition of the document_list_widget class.

#pragma once

#include "cxxdbg/app/forward.hpp"
#include "cxxdbg/util/signals.hpp"
#include <filesystem>
#include <map>
#include <QFrame>
#include <QWidget>


class QTabWidget;


namespace cxxdbg::gui {

class document_widget;
class document_widget_factory;


/// Widgets that displays editors/viewers for all opened documents in tabs
class document_list_widget: public QFrame {
    Q_OBJECT
public:
    /// Constructs document list widget with specified reference to document list,
    /// pointer to document widget factory function, and parent widget
    explicit document_list_widget(cxxdbg::document_list & docs,
                                  const std::shared_ptr<document_widget_factory> & doc_widget_factory,
                                  QWidget * parent = NULL);

    /// Sets font for displaying text in documents
    void setTextFont(const QFont & font);

    /// Sets tab size for displaying text in documents
    void setTabSize(unsigned sz);

    /// Highlights search string in active source code widget
    void highlightSearchString(const QString & str, bool matchCase);

    /// Removes highlighting for search string
    void removeSearchStringHighlight();

    /// Searches next occurence of search string in active source code widget
    void searchNext();

    /// Searches previous occurence of search string in active
    /// source code widget
    void searchPrev();
    
    // shows goto line number dialog
    void gotoLineNumber();

    /// Returns true if can do search next / prev
    bool canDoSearch() const;
    
    /// Displays next opened tab
    void displayNextTab();
    
    /// Displays previous opened tab
    void displayPrevTab();

signals:
    /// Called when user selects open source in menu
    void openSelected();

    /// Called when canDoSearch property changed
    void canDoSearchChanged();

private:
    /// Called after document was opened
    void onDocumentOpened(cxxdbg::document * doc);

    /// Called before document was closed
    void onDocumentClosed(cxxdbg::document * doc);

    /// Called when current source file is changed
    void onCurrentSourceChanged();

    /// Updates document name in tab
    void updateDocumentName(const cxxdbg::document * doc);

    /// Returns active document widget
    document_widget * activeDocumentWidget();

    /// Updates style depending on number of tab widgets
    void updateStyle();


    /// Type of map from document to document widget
    typedef std::map<const cxxdbg::document*, document_widget*> DocumentWidgetMap;

    cxxdbg::document_list & docs_;                                    ///< Reference to document list
    std::shared_ptr<document_widget_factory> doc_widget_factory_;   ///< Reference to document widget factory
    QTabWidget * tabs_;                                             ///< Tabs in central widget
    DocumentWidgetMap sourceWidgets_;                               ///< Map from documents to widgets
    QString searchString_;                                          ///< Current search string
    bool matchCase_;                                                ///< Current match case property
    bool searchHighlight_;                                          ///< Highlight search string?
    QFont textFont_;                                                ///< Font for displaying text in documents
    unsigned tabSize_;                                              ///< Tab size in text documents

    /// Connection to source file opened signal
    cxxdbg::util::scoped_signal_connection source_file_opened_con_;

    /// Connection to source file closed signal
    cxxdbg::util::scoped_signal_connection source_file_closed_con_;

    /// Connection to current source file changed signal
    cxxdbg::util::scoped_signal_connection current_source_changed_con_;

    cxxdbg::util::scoped_signal_connection document_name_changed_con_;
    cxxdbg::util::scoped_signal_connection document_changed_changed_con_;
};


}
