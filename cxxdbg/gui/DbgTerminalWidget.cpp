// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "DbgTerminalWidget.h"
#include "Utils.h"
#include "cxxdbg_connect.hpp"
#include "cxxdbg/dbg/debugger.hpp"
#include "cxxdbg/cli/app_command_interpreter.hpp"

#include <QEvent>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QMainWindow>


namespace cxxdbg::gui {


DbgTerminalWidget::DbgTerminalWidget(cxxdbg::dbg::debugger & dbg,
                                     cxxdbg::dbg::debugger_terminal & dbg_term,
                                     cxxdbg::dbg::cli::app_command_interpreter & interp,
                                     QWidget * autoCompleteParent,
                                     QWidget * parent):
QWidget(parent),
dbg_{dbg},
dbg_term_{dbg_term},
//model_{interp},
//qtModel_{model_},
cursor_position_{0} {
    auto * lay = new QVBoxLayout(this);
    this->setLayout(lay);
    lay->setContentsMargins({});
    lay->setSpacing(0);

    output_ = new QTextEdit(this);
    lay->addWidget(output_);
    output_->setFrameStyle(QFrame::NoFrame);
    output_->setReadOnly(true);
    output_->setContextMenuPolicy(Qt::DefaultContextMenu);
    output_->setFont(defaultFixedFont());

    {
        auto sep = new QFrame;
        lay->addWidget(sep);
        sep->setFrameStyle(QFrame::Plain);
        sep->setFrameShape(QFrame::HLine);

        auto p = palette();
        p.setColor(QPalette::WindowText, p.color(QPalette::Mid));
        sep->setPalette(p);
    }

    cmdLine_ = new QLineEdit(this);
    cmdLine_->setFrame(false);
    lay->addWidget(cmdLine_);

    cmdLine_->setFont(defaultFixedFont());

    // handling enter
    cxxdbg_connect(cmdLine_, &QLineEdit::returnPressed, [this] { onReturnPressed(); });

    // handling key events
    cmdLine_->installEventFilter(this);

    // displaying process exit message in terminal
    pexit_con_ = dbg_.process_exited().connect([this](int ecode) {
        output_->append(tr("Process exited with exit code %1\n").arg(QString::number(ecode)));
    });

    auto * mainWindow = static_cast<QWidget*>(static_cast<QMainWindow*>(autoCompleteParent));
    //auto * model = static_cast<QAbstractItemModel *>(&qtModel_);
    //completionWidget_ = new AutoCompletionWidget(model, mainWindow);

    // can become more complex
    auto calculate_prefix = [this] () -> std::string {
        return this->text_.mid(0, this->cursor_position_).toStdString();
    };

#if 0
    auto update_model = [this, calculate_prefix]() {
        auto prefix = calculate_prefix();
        model_.handle_prefix_changed()(prefix);
        auto size = model_.childs_size({});
        bool is_visible = size > 0 && prefix.length() > 0;
        if (is_visible) {
            completionWidget_->showWidget(cmdLine_->mapToGlobal({0, 0}), AutoCompletionWidget::Above, 6);
        } else {
            completionWidget_->hideWidget();
        }
    };
#endif

    cxxdbg_connect(cmdLine_, &QLineEdit::textChanged, [this](const QString & text) {
        text_ = text;
//        update_model();
    });

    cxxdbg_connect(cmdLine_, &QLineEdit::cursorPositionChanged, [this] (int oldPos, int newPos) {
        cursor_position_ = newPos;
//        update_model();
    });

#if 0
    cxxdbg_connect(completionWidget_, &AutoCompletionWidget::itemDoubleClicked, [this](const QModelIndex & index) {
        if (!index.isValid())
            return;

        auto text = qtModel_.data(index, Qt::DisplayRole).toString();
        cmdLine_->setText(text + QChar(' '));
    });

    completionWidget_->hideWidget();
#endif

    output_con_ = dbg_term_.output_received.connect([this](const std::string & data) {
        output_->append(QString::fromStdString(data));
    });
}


void DbgTerminalWidget::showEvent(QShowEvent * event) {
    QWidget::showEvent(event);
    cmdLine_->setFocus();
}


void DbgTerminalWidget::onReturnPressed() {
    QString cmdText = cmdLine_->text();
    dbg_term_.exec(cmdLine_->text().toStdString(), [](auto && ...) {});
    cmdLine_->clear();
}


bool DbgTerminalWidget::eventFilter(QObject * obj, QEvent * event) {
    if (obj != cmdLine_) {
        return false;
    }

    if (event->type() != QEvent::KeyPress)
        return false;

    auto * focusEvent = static_cast<QFocusEvent*>(event);
    // if (focusEvent && focusEvent->lostFocus()) {
    //     completionWidget_->hideWidget();
    // }

    QKeyEvent * kevent = static_cast<QKeyEvent*>(event);

    std::string cmd;

    if (kevent) {
        // if (kevent->key() == Qt::Key_Escape) {
        //     completionWidget_->hideWidget();
        // }

        if (kevent->key() == Qt::Key_Up) {
            cmd = dbg_term_.prev_cmd();
        } else if (kevent->key() == Qt::Key_Down) {
            cmd = dbg_term_.next_cmd();
        } else {
            return false;
        }
    }

    if (!cmd.empty()) {
        cmdLine_->setText(QString::fromStdString(cmd));
    }
    return true;
}


}
