// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file TerminalWidget.h
/// Contains definition of the TerminalWidget class.

#pragma once

#include <QPlainTextEdit>
#include "cxxdbg/app/signals.hpp"


class QTimer;

namespace cxxdbg {
    class terminal;
}


namespace cxxdbg::gui {


/// Terminal emulator widget
class TerminalWidget: public QPlainTextEdit {
public:
    /// Constructs terminal widget with specified reference to application
    /// executable terminal
    TerminalWidget(cxxdbg::terminal & term);

private:
    /// Updates state of text edit widget
    void updateTextEditState();

    /// Adds text to text eidt using default color
    void addText(const std::wstring & data);

    /// Adds text to text edit using specified color
    void addText(const std::wstring & data, const QColor & col);

    /// Shows or hides cursor
    void showCursor(bool show);

    /// Enables / disables cursor and resets its state
    void enableCursor(bool enable);

    /// Handler for key press event
    void keyPressEvent(QKeyEvent * event) override;

    /// Handler for focus in event
    void focusInEvent(QFocusEvent * event) override;

    /// Handler for focus out event
    void focusOutEvent(QFocusEvent * event) override;


    cxxdbg::terminal & term_;

    /// True if cursor symbol is visible when blinking
    bool cursor_visible_ = true;

    QTimer * cursorTimer_;      ///< Timer for cursor blinking

    /// Connection to can_send_stdin changed signal in terminal
    cxxdbg::scoped_signal_connection can_send_stdin_changed_con_;

    /// Connection to stdout received signal in terminal
    cxxdbg::scoped_signal_connection stdout_received_con_;

    /// Connection to stderr received signal in terminal
    cxxdbg::scoped_signal_connection stderr_received_con_;
};


}
