// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file debug_source_code_widget_delegate.hpp
/// Contains definition of the debug_source_code_widget_delegate class.

#pragma once

#include "text_document_widget.hpp"
#include "cxxdbg/dbg/forward.hpp"


namespace cxxdbg::gui {


class DebugUI;


/// Delegate for source code widgets in debugger
class debug_source_code_widget_delegate: public text_document_widget_delegate {
public:
    /// Constructs delegate with specified reference to debugger object
    debug_source_code_widget_delegate(dbg::debugger & dbg, DebugUI & dbgUi);

    /// Calculates list of iconst that should be painted at specified line
    std::list<QIcon> calculate_line_icons(const text_document * doc, size_t line) override;

    /// Calculates list of highlighted lines for specified document
    std::list<std::pair<size_t, QColor>> calculate_highlighted_lines(const text_document * doc) override;

    /// Returns description that should be displayed in tooltip for marks area
    /// at specified line
    std::string get_line_description(const text_document * doc, size_t line) override;

    /// Handles click on marks area at specified line
    void handle_line_click(const text_document * doc, size_t line) override;

    /// Creates context menu for marks area at specified line
    std::unique_ptr<QMenu> create_marks_area_context_menu(const text_document * doc, size_t line) override;

    /// Adds menu items at the end of code viewer/editor context menu for specified line
    void add_text_context_menu_actions(QMenu * menu,
                                       text_document * doc,
                                       size_t line,
                                       const std::wstring & selText) override;

private:
    /// Returns true if there is thread that has current position at
    /// specified line of document
    bool isThreadAtLine(const dbg::source_file * file, unsigned int line);

    /// Returns true if current thread has position at specified line of document
    bool isCurrentThreadAtLine(const dbg::source_file * file, unsigned int line);

    /// Returns true if current frame has position at specified line of document
    bool isCurrentFrameAtLine(const dbg::source_file * file, unsigned int line);

    /// Creates marks area context menu for breakpoint
    std::unique_ptr<QMenu> createMarksAreaBreakpointMenu(const dbg::code_breakpoint * bp);

    /// Creates marks area context menu for thread
    std::unique_ptr<QMenu> createMarksAreaThreadMenu(const dbg::thread * thrd);

    /// Creates Breakpoint menu for context menu for text widget
    void createTextWidgetBreakpointMenu(QMenu * parent,
                                        const dbg::source_file * file,
                                        unsigned int line);

    /// Creates Thread menu for context menu for text widget
    void createTextWidgetThreadMenu(QMenu * parent,
                                    const dbg::source_file * file,
                                    unsigned int line);

    dbg::debugger & dbg_;           ///< Reference to debugger object
    DebugUI & dbgUi_;               ///< Reference to common debug UI object

    util::scoped_signal_connection state_changed_con_;
    util::scoped_signal_connection current_frame_changed_con_;
    util::scoped_signal_connection breakpoints_changed_con_;
};


}
