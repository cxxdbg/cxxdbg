// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include <QWidget>

#pragma once


class QLineEdit;
class QTextEdit;

namespace cxxdbg::dbg {
    namespace cli {
        class app_command_interpreter;
    }

    class debugger;
    class debugger_terminal;
}


namespace cxxdbg::gui {


class DbgTerminalWidget: public QWidget {
    Q_OBJECT

public:
    /// Constructor, makes terminal widget with specified debugger and
    /// debugger terminal and interpreter instances
    explicit DbgTerminalWidget(cxxdbg::dbg::debugger & dbg,
                               cxxdbg::dbg::debugger_terminal & dbg_term,
                               cxxdbg::dbg::cli::app_command_interpreter & interp,
                               QWidget * autoCompleteParent,
                               QWidget * parent = nullptr);

protected:
    void showEvent(QShowEvent * event) override;

private:
    /// Called when user press return on command line
    void onReturnPressed();

    /// Filter for handling key events on line edit
    bool eventFilter(QObject * obj, QEvent * event) override;

    cxxdbg::dbg::debugger & dbg_;                         ///< Reference to debugger instance
    cxxdbg::dbg::debugger_terminal & dbg_term_;           ///< Reference to debugger terminal instance
    QLineEdit * cmdLine_;                               ///< Command line input
    QTextEdit * output_;                                ///< Output
    cxxdbg::scoped_signal_connection pexit_con_;          ///< Connection to process exited signal
    cxxdbg::scoped_signal_connection output_con_;         ///< Connection to debugger terminal output signal

    //cxxdbg::dbg::auto_completion_model model_;            ///< Auto completion model

    //AppRoItemModel qtModel_;                            ///< Qt call stack model
    //AutoCompletionWidget * completionWidget_;           ///< Command auto completion widget

    int cursor_position_;                               ///< Cursor position
    QString text_;                                      ///< Text
};


}
