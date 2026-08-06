// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "document_widget.hpp"
#include "cxxdbg_plain_document_view.hpp"
#include "cxxdbg/app/forward.hpp"
#include "cxxdbg/util/signals.hpp"
#include <QFrame>
#include <QTextEdit>
#include <QWidget>
#include <vector>
#include <list>


class QMenu;


namespace cxxdbg::gui {

template <
    tmvc::text_model TextModel,
    tmvc::selection_model SelectionModel,
    tmvc::selection_controller_for<TextModel> Controller
>
class DocumentMarksWidget;

class LineNumbersWidget;
class CxxdbgApplication;


/// Abstract class for customizing text document widget.
/// Contains methods for displaying icons and context menu on marks panel,
/// and for displaying context menu in code editor/viewer
class text_document_widget_delegate: public QObject {
    Q_OBJECT

public:
    /// Virtual destructor
    virtual ~text_document_widget_delegate() = default;

    /// Calculates list of icons that should be painted at specified line
    virtual std::list<QIcon> calculate_line_icons(const text_document * doc, size_t line) = 0;

    /// Calculates list of highlighted lines for specified document
    virtual std::list<std::pair<size_t, QColor>> calculate_highlighted_lines(const text_document * doc) = 0;

    /// Returns description that should be displayed in tooltip for marks area
    /// at specified line
    virtual std::string get_line_description(const text_document * doc, size_t line) = 0;

    /// Handles click on marks area at specified line
    virtual void handle_line_click(const text_document * doc, size_t line) = 0;

    /// Creates context menu for marks area at specified line
    virtual std::unique_ptr<QMenu> create_marks_area_context_menu(const text_document * doc, size_t line) = 0;

    /// Adds menu items at the end of code viewer/editor context menu for specified line
    virtual void add_text_context_menu_actions(QMenu * menu,
                                               text_document * doc,
                                               size_t line,
                                               const std::wstring & selText) = 0;

    /// This signal should be emitted by delegate when marks area should
    /// be updated in source code widget
    mutable util::signal<void()> mark_area_should_be_updated;

    /// The signal is emitted when highlighted lines are changed
    mutable cxxdbg::util::signal<void()> highlighted_lines_changed;
};


/// Base class for all text document widget instantiations
class text_document_widget_base: public document_widget {
public:
    /// Constructs widget with specified parent
    explicit text_document_widget_base(QWidget * parent = nullptr):
        document_widget{parent} {}

    /// Returns width of line numbers widget
    virtual int linesNumbersWidth() const = 0;

    /// Handles paint event in line numbers widget
    virtual void handleLineNumbersPaint(QPaintEvent * event) = 0;

    /// Handles mouse move event in line numbers widget
    virtual void handleLineNumbersMouseMove(QMouseEvent * event) = 0;

    /// Handles mouse click event in line numbers widget
    virtual void handleLineNumbersClick(QMouseEvent * event) = 0;
};


/// Widget for displaying/editing documents represented as text (sources, memory, disassembly)
template <
    tmvc::text_model TextModel,
    tmvc::selection_model SelectionModel,
    tmvc::selection_controller_for<TextModel> Controller
>
class text_document_widget: public text_document_widget_base {
    /// Type of text document
    using text_document_t = text_model_document<TextModel, SelectionModel, Controller>;

    /// Type of document view
    using text_document_view = cxxdbg_plain_document_view<TextModel, SelectionModel, Controller>;

    /// Type of document marks widget
    using document_marks_widget_t = DocumentMarksWidget<TextModel, SelectionModel, Controller>;

    friend class LineNumbersWidget;
    friend document_marks_widget_t;

public:
    /// Constructor, makes source code widget with specified source file and parent
    explicit text_document_widget(text_document_t * doc,
                                  text_document_widget_delegate * delegate,
                                  QWidget *parent = 0);

    /// Returns pointer to document associated with this widget
    text_document_t * doc() override { return doc_; }

    /// Highlights search string in source code
    void highlight_search_string(const QString & str, bool match_case) override;

    /// Removes highlighting for search string
    void remove_search_string_highlight() override;

    /// Searches next occurence of search string
    void search_next(const QString & str, bool match_case) override;

    /// Searches previous occurence of search string
    void search_prev(const QString & str, bool match_case) override;

    /// Sets font in widget
    void set_font(const QFont & font) override;

    /// Sets tab size in widget
    void set_tab_size(unsigned sz) override;


private:
    /// Handles paint event in document marks widget
    void handleDocumentMarksPaint(QPaintEvent * event);

    /// Handles mouse move event in document marks widget
    void handleDocumentMarksMouseMove(QMouseEvent * event);

    /// Handles mouse click event in document marks widget
    void handleDocumentMarksClick(QMouseEvent * event);

    /// Handles paint event in line numbers widget
    void handleLineNumbersPaint(QPaintEvent * event) override;

    /// Handles mouse move event in line numbers widget
    void handleLineNumbersMouseMove(QMouseEvent * event) override;

    /// Handles mouse click event in line numbers widget
    void handleLineNumbersClick(QMouseEvent * event) override;

    /// Returns line number from mouse position
    int lineNumberFromMousePos(const QPoint & pos);

    /// Paints line numbers in line numbers widget
    void paintLineNumbers(QPainter & painter);

    /// Paints icon at specified line
    void paintIcon(QPainter & painter, unsigned int line, QIcon icon);

    /// Returns width of document marsk widget
    int documentMarksWidth() const;

    /// Returns width of line numbers widget
    int linesNumbersWidth() const override;

    /// Returns number of first visible line
    int firstVisibleLine() const;

    /// Returns number of last visible line
    int lastVisibleLine() const;

    /// Returns vertical position of line with specified number
    int linePos(int lineNum) const;

    /// Called when user requiests line numbers context menu
    void onDocumentMarksContextMenu(const QPoint & pos);

    /// Adds custom menu items into context menu
    void addTextWidgetContextMenuItems(QMenu * menu);

    /// Updates highlights for text widget
    void updateHighlights();


    text_document_t * doc_;                             ///< Text document in application model
    document_marks_widget_t * documentMarks_;           ///< Document marks area
    LineNumbersWidget * lineNumbers_;                   ///< Line nubmers area
    text_document_view * textWidget_;                   ///< Source code text widget
    std::vector<QTextCursor> searchResults_;            ///< Vector of search result selections
    bool searchHighlights_;                             ///< Show show search highlights?
    text_document_widget_delegate * delegate_;          ///< Pointer to custom delegate

    /// Connection to state changed signal
    cxxdbg::util::scoped_signal_connection state_changed_con_;

    /// Connection to current frame changed signal
    cxxdbg::util::scoped_signal_connection current_frame_changed_con_;

    /// Connection to breakpoints changed signal
    cxxdbg::util::scoped_signal_connection breakpoints_changed_con_;

    /// Connection to delegate mark area update signal
    cxxdbg::util::scoped_signal_connection mark_area_update_con_;

    /// Connection to delegate highlighted lines changed signal
    cxxdbg::util::scoped_signal_connection highlighted_lines_changed_con_;
};


}


#include "text_document_widget.ipp"
