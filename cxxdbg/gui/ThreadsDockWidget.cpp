// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file ThreadsDockWidget.cpp
/// Contains implementation of ThreadsDockWidget class.

#include "ThreadsDockWidget.h"
#include "DebugUI.h"
#include "CxxdbgTreeWidget.h"
#include "cxxdbg/dbg/debugger.hpp"
#include "cxxdbg/app/document_navigator.hpp"
#include "cxxdbg/dbg/source_file.hpp"
#include "cxxdbg/dbg/stack_frame.hpp"
#include "cxxdbg/dbg/thread.hpp"
#include "cxxdbg/dbg/thread_list.hpp"
#include "cxxdbg_connect.hpp"

#include <ranges.hpp>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QToolBar>
#include <QVBoxLayout>


namespace fs = std::filesystem;


Q_DECLARE_METATYPE(const cxxdbg::dbg::thread*)


namespace cxxdbg::gui {


ThreadsDockWidget::ThreadsDockWidget(cxxdbg::dbg::debugger & dbg,
                                     document_navigator & doc_nav,
                                     DebugUI & dbgUi,
                                     QWidget * parent) :
CxxdbgDockWidget(tr("Threads"), parent),
dbg_{dbg},
doc_nav_{doc_nav},
dbgUi_{dbgUi} {
    setObjectName("threads");

    // switch to thread action
    selectThreadAction_ = new QAction{dbgUi.makeSvgIcon("select"), tr("Switch to thread"), this};
    selectThreadAction_->setEnabled(false);
    cxxdbg_connect(selectThreadAction_, &QAction::triggered, [this]() {
        // getting selected item
        QTreeWidgetItem * item = threads_->currentItem();
        assert(item != nullptr && "Selected item is null");

        // getting thread from selected item
        const cxxdbg::dbg::thread * thread =
                item->data(0, Qt::UserRole).value<const cxxdbg::dbg::thread*>();
        assert(thread != nullptr && "thread value for selected item is null");

        dbg_.set_current_thread(thread);
    });

    // go to source action
    QIcon goToSourceIcon = dbgUi.makeSvgIcon("goto");
    goToSourceAction_ = new QAction{goToSourceIcon, tr("Go to source"), this};
    goToSourceAction_->setEnabled(false);
    cxxdbg_connect(goToSourceAction_, &QAction::triggered, [this]() {
        // getting selected item
        QTreeWidgetItem * item = threads_->currentItem();
        assert(item != nullptr && "Selected item is null");

        // getting thread from selected item
        const cxxdbg::dbg::thread * thread =
                item->data(0, Qt::UserRole).value<const cxxdbg::dbg::thread*>();
        assert(thread != nullptr && "thread value for selected item is null");

        // show source position
        auto path = thread->pos().src_pos().file()->path();
        auto line = static_cast<size_t>(thread->pos().src_pos().line() - 1);
        doc_nav_.show_pos(path, line);
    });

    // making thread list
    threads_ = new CxxdbgTreeWidget("ui/threads/tree");
    setWidget(threads_);
    threads_->setFrameStyle(QFrame::NoFrame);
    QStringList headerLabels;
    headerLabels.append(QString());
    headerLabels.append(tr("Thread ID"));
    headerLabels.append(tr("Location"));
    threads_->setHeaderLabels(headerLabels);
    threads_->setColumnWidth(0, fontMetrics().height() + 4);
    threads_->setRootIsDecorated(false);
    threads_->setSelectionMode(QAbstractItemView::ContiguousSelection);
    threads_->restoreState();

    // copy action
    copyAction_ = new QAction(QIcon::fromTheme("edit-copy"), tr("Copy"), this);
    cxxdbg_connect(copyAction_, &QAction::triggered, [this] { onCopy(); });


    // select all action
    QAction * selectAllAction = new QAction(QIcon::fromTheme("edit-select-all"), "Select all", this);
    cxxdbg_connect(selectAllAction, &QAction::triggered, [this](bool) {
        threads_->selectAll();
    });


    // hexadecimal display action
    hexDisplayAction_ = new QAction(tr("Hexadecimal display"), this);
    hexDisplayAction_->setCheckable(true);
    cxxdbg_connect(hexDisplayAction_, &QAction::triggered, [this](bool val) { onHexDisplay(val); });

    // adding context menu for thread list

    threads_->addAction(copyAction_);
    threads_->addAction(selectAllAction);

    threads_->addSeparator();

    threads_->addAction(hexDisplayAction_);
    threads_->addAction(dbgUi_.showThreadsInSourceAction());

    threads_->addSeparator();

    threads_->addAction(selectThreadAction_);
    threads_->addAction(goToSourceAction_);
    threads_->setContextMenuPolicy(Qt::ActionsContextMenu);


    // adding context menu for list header
    threads_->addColumnSelectContextMenu();

    // handling double click on item in thread list
    cxxdbg_connect(threads_, &QTreeWidget::itemDoubleClicked, [this](QTreeWidgetItem *, int) {
        if (selectThreadAction_->isEnabled())
            selectThreadAction_->trigger();
    });

    // enable actions only if selection is not empty
    cxxdbg_connect(threads_, &QTreeWidget::itemSelectionChanged, [this]() {
        QList<QTreeWidgetItem*> selectedItems = threads_->selectedItems();

        copyAction_->setEnabled(!selectedItems.empty());
        selectThreadAction_->setEnabled(selectedItems.size() == 1);

        // enable go to source action if selected single thread and it
        // it's source position can be shown
        if (selectedItems.size() != 1) {
            goToSourceAction_->setEnabled(false);
        } else {
            // getting thread from selected item
            const cxxdbg::dbg::thread * thread =
                    selectedItems.at(0)->data(0, Qt::UserRole).value<const cxxdbg::dbg::thread*>();
            assert(thread != nullptr && "thread value for selected item is null");

            // checking that source position is available
            if (thread->pos().src_pos()) {
                // checking thread pos
                auto exists = fs::exists(thread->pos().src_pos().file()->path());
                goToSourceAction_->setEnabled(exists);
            } else {
                goToSourceAction_->setEnabled(false);
            }
        }
    });


    // connecting to state changed signal
    stateChangedCon_ = dbg_.state_changed().connect([this]() { onStateChanged(); });

    // connecting to current thread changed signal
    currentThreadChangedCon_ = dbg_.current_thread_changed().connect([this]() { onCurrentThreadChanged(); });

    // updating widget
    onStateChanged();
}


void ThreadsDockWidget::saveState() {
    threads_->saveState();
}


QString ThreadsDockWidget::threadIdToString(unsigned long id) {
    std::wostringstream str;
    if (hexDisplayAction_->isChecked()) {
        str << L"0x" << std::hex << std::setw(8) << std::setfill(L'0') << id;
    } else {
        str << id;
    }

    return QString::fromStdWString(str.str()).toUpper();
}


void ThreadsDockWidget::onStateChanged() {
    if (dbg_.state() != cxxdbg::dbg::debugger::state_t::stopped) {
        // clear thread list and disable widget
        threads_->clear();
        setContentEnabled(false);
        return;
    }

    setContentEnabled(true);

    // add threads in dock widget
    for (auto it = dbg_.threads().begin(), end = dbg_.threads().end(); it != end; ++it) {

        const cxxdbg::dbg::thread * thread = *it;

        QStringList columns;

        // adding column with 'current' marker
        columns.append(QString());

        // adding column with thread id
        columns.append(threadIdToString(thread->id()));

        // adding column with thread location
        if (!std::ranges::empty(thread->call_stack())) {
            const auto & cstack = thread->call_stack();
            const cxxdbg::dbg::stack_frame * top_frame = *std::ranges::begin(cstack);

            if (!top_frame->pos().func_name().empty()) {
                // display function name in location
                columns.append(QString::fromStdString(top_frame->pos().func_name()));
            } else {
                // display address in location
                columns.append(QString::number(top_frame->pos().load_addr(), 16));
            }

        } else {
            columns.append(tr("<empty call stack>"));
        }

        QTreeWidgetItem * item = new QTreeWidgetItem(columns);
        item->setData(0, Qt::UserRole, QVariant::fromValue(thread));

        // setting icon for current thread
        if (thread == dbg_.current_thread()) {
            item->setIcon(0, dbgUi_.rightYellowIcon());
        }

        threads_->addTopLevelItem(item);
    }
}


void ThreadsDockWidget::onCurrentThreadChanged() {
    const cxxdbg::dbg::thread * cur_thread = dbg_.current_thread();

    // searching for item in tree widget
    for (int i = 0, e = threads_->topLevelItemCount(); i < e; ++i) {
        QTreeWidgetItem * item = threads_->topLevelItem(i);
        const cxxdbg::dbg::thread * item_thread =
                item->data(0, Qt::UserRole).value<const cxxdbg::dbg::thread*>();

        if (item_thread == cur_thread) {
            item->setIcon(0, dbgUi_.rightYellowIcon());
        } else {
            item->setIcon(0, QIcon());
        }
    }
}


void ThreadsDockWidget::onCopy() {
    threads_->copySelectedToClipboard();
}


void ThreadsDockWidget::onHexDisplay(bool) {

    // change ID column value for all items
    for (int i = 0, e = threads_->topLevelItemCount(); i < e; ++i) {
        QTreeWidgetItem * item = threads_->topLevelItem(i);
        const cxxdbg::dbg::thread * thread =
                item->data(0, Qt::UserRole).value<const cxxdbg::dbg::thread*>();
        item->setText(1, threadIdToString(thread->id()));
    }
}


}
