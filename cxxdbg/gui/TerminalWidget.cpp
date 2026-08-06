// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file TerminalWidget.cpp
/// Contains implementation of the TerminalWidget class.

#include "TerminalWidget.h"
#include "Utils.h"
#include "cxxdbg_connect.hpp"
#include "cxxdbg/app/terminal.hpp"
#include <sstream>
#include <QFontDatabase>
#include <QScrollBar>
#include <QTimer>


namespace cxxdbg::gui {


TerminalWidget::TerminalWidget(cxxdbg::terminal & term):
term_{term} {
    setTextInteractionFlags(Qt::TextSelectableByMouse);
    setFont(defaultFixedFont());

    // adding _ symbol at the end of
    // text edit to emulate terminal cursor
    {
        auto cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
        insertPlainText("_");
    }

    // make cursor invisible after creating widget
    showCursor(false);

    // setting up timer for cursor blinking
    cursorTimer_ = new QTimer(this);
    cxxdbg_connect(cursorTimer_, &QTimer::timeout, [this] {
        showCursor(!cursor_visible_);
    });


    // listening for change of can_send_stdin in terminal
    can_send_stdin_changed_con_ = term_.can_send_stdin_changed().connect([this] {
        updateTextEditState();
    });

    // listening for stdout
    stdout_received_con_ = term_.stdout_received().connect([this](const std::wstring & data) {
        addText(data);
    });

    // listening for stderr
    stderr_received_con_ = term_.stderr_received().connect([this](const std::wstring & data) {
        addText(data);
    });

    // listening for process start
    term_.started().connect([this](auto && cmd) {
        // making all output gray
        auto curs = this->textCursor();
        curs.movePosition(QTextCursor::End);
        curs.movePosition(QTextCursor::Left);
        curs.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
        QTextCharFormat fmt;
        const auto & col = palette().color(QPalette::Disabled, QPalette::WindowText);
        fmt.setForeground(col);
        curs.setCharFormat(fmt);

        // scrolling output to the end
        verticalScrollBar()->setSliderPosition(verticalScrollBar()->maximum());

        if (!cmd.empty()) {
            // adding process start message
            std::wostringstream msg;
            msg << L"Starting command: '" << cmd << "'\n";
            addText(msg.str(), QColor{0, 0, 255});
        }
    });

    // listening for process exit
    term_.exited().connect([this](int ecode) {
        // adding process exit message
        std::wostringstream msg;
        msg << L"Process exited with exit code " << ecode << "\n";
        addText(msg.str(), QColor{0, 0, 255});
    });

    // listening for launch error
    term_.start_error().connect([this](const std::wstring & err) {
        addText(err + L"\n", QColor{255, 0, 0});
    });

    updateTextEditState();
}


void TerminalWidget::updateTextEditState() {
    enableCursor(hasFocus() && term_.can_send_stdin());
}


void TerminalWidget::addText(const std::wstring & data) {
    addText(data, palette().color(QPalette::WindowText));
}


void TerminalWidget::addText(const std::wstring & data, const QColor & col) {
    // detect if terminal is scrolled to the end
    auto sb = verticalScrollBar();
    bool atEnd = sb->sliderPosition() == sb->maximum();

    auto cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.movePosition(QTextCursor::Left);
    QTextCharFormat fmt;
    fmt.setForeground(col);
    cursor.setCharFormat(fmt);
    cursor.insertText(QString::fromStdWString(data));

    // scrolling to the end if we was in the end
    if (atEnd) {
        sb->setSliderPosition(sb->maximum());
    }

    // reseting cursor and enabling it if widget has focus
    updateTextEditState();
}


void TerminalWidget::showCursor(bool show) {
    auto cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);

    QTextCharFormat fmt;
    auto palColor = show ? QPalette::WindowText : QPalette::Base;
    fmt.setForeground(palette().color(palColor));
    cursor.setCharFormat(fmt);

    cursor_visible_ = show;
}


void TerminalWidget::enableCursor(bool enable) {
    // stopping timer
    cursorTimer_->stop();

    if (!enable) {
        // hiding cursor
        showCursor(false);
        return;
    }


    // making cursor visible
    showCursor(true);

    // starting timer
    cursorTimer_->start(500);
}


void TerminalWidget::keyPressEvent(QKeyEvent * event) {
    if (!term_.can_send_stdin()) {
        return;
    }

    auto data = event->text().toLocal8Bit().toStdString();
    term_.send_stdin(data);

    event->accept();

    // display cursor after text insert
    enableCursor(true);
}


void TerminalWidget::focusInEvent(QFocusEvent * event) {
    QPlainTextEdit::focusInEvent(event);
    updateTextEditState();
}


void TerminalWidget::focusOutEvent(QFocusEvent * event) {
    QPlainTextEdit::focusOutEvent(event);
    updateTextEditState();
}


}
