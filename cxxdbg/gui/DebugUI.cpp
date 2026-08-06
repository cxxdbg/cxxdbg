// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file DebugUI.cpp
/// Contains implementation of the DebugUI class.

#include "DebugUI.h"
#include "AddWatchpointDialog.hpp"
#include "BreakpointConditionDialog.h"
#include "BreakpointHitCountDialog.h"
#include "EditAddressBreakpointDialog.h"
#include "EditFunctionBreakpointDialog.h"
#include "EditSourcePosBreakpointDialog.h"
#include "FunctionNameDialog.h"
#include "StepUntilDialog.h"
#include "Utils.h"
#include "cxxdbg_connect.hpp"
#include "cxxdbg/app/document_list.hpp"
#include "cxxdbg/app/text_file_document.hpp"
#include "cxxdbg/dbg/breakpoint_list.hpp"
#include "cxxdbg/dbg/debugger.hpp"
#include "cxxdbg/dbg/source_model.hpp"
#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QStyle>


namespace fs = std::filesystem;


namespace cxxdbg::gui {


DebugUI::DebugUI(cxxdbg::dbg::debugger & dbg,
                 cxxdbg::document_list & docList,
                 QObject * parent,
                 const cxxdbg::ro_tree_view_model * srcList):
dbg_{dbg},
docList_{docList},
parent_{parent},
srcList_{srcList} {
    initIcons();
    initActions();
    updateActionsStatus();

    // updating actions status when debugger state changed
    app_state_changed_con_ = dbg_.state_changed().connect([this]() {
        updateActionsStatus();
    });
}


void DebugUI::initActions() {
    // Continue action
    debugContinueAction_ = new QAction(QObject::tr("Continue"), parent_);
    debugContinueAction_->setShortcut(QKeySequence(QObject::tr("F5")));
    debugContinueAction_->setIcon(debugContinueIcon_);
    cxxdbg_connect(debugContinueAction_, &QAction::triggered, [this] { dbg_.run(); });

    // Interrupt action
    debugInterruptAction_ = new QAction(QObject::tr("Interrupt"), parent_);
    debugInterruptAction_->setIcon(debugInterruptIcon_);
    cxxdbg_connect(debugInterruptAction_, &QAction::triggered, [this] { dbg_.stop(); });

    // Step into action
    debugStepIntoAction_ = new QAction(QObject::tr("Step into"), parent_);
#ifdef __APPLE__
    debugStepIntoAction_->setShortcut(QKeySequence(QObject::tr("F7")));
#else
    debugStepIntoAction_->setShortcut(QKeySequence(QObject::tr("F11")));
#endif
    debugStepIntoAction_->setIcon(debugStepIntoIcon_);
    cxxdbg_connect(debugStepIntoAction_, &QAction::triggered, [this] { dbg_.step_into(); });

    // Step over action
    debugStepOverAction_ = new QAction(QObject::tr("Step over"), parent_);
#ifdef __APPLE__
    debugStepOverAction_->setShortcut(QKeySequence(QObject::tr("F6")));
#else
    debugStepOverAction_->setShortcut(QKeySequence(QObject::tr("F10")));
#endif
    debugStepOverAction_->setIcon(debugStepOverIcon_);
    cxxdbg_connect(debugStepOverAction_, &QAction::triggered, [this] { dbg_.step_over(); });

    // Step out action
    debugStepOutAction_ = new QAction(QObject::tr("Step out"), parent_);
#ifdef __APPLE__
    debugStepOutAction_->setShortcut(QKeySequence(QObject::tr("F8")));
#else
    debugStepOutAction_->setShortcut(QKeySequence(QObject::tr("F12")));
#endif
    debugStepOutAction_->setIcon(debugStepOutIcon_);
    cxxdbg_connect(debugStepOutAction_, &QAction::triggered, [this] { dbg_.step_out(); });

    // Step instruction into action
    debugStepInstIntoAction_ = new QAction(QObject::tr("Step instruction into"), parent_);
    cxxdbg_connect(debugStepInstIntoAction_, &QAction::triggered, [this] {
        dbg_.step_inst_into();
    });

    // Step instruction over action
    debugStepInstOverAction_ = new QAction(QObject::tr("Step instruction over"), parent_);
    cxxdbg_connect(debugStepInstOverAction_, &QAction::triggered, [this] {
        dbg_.step_inst_over();
    });

    // Step until action
    debugStepUntilAction_ = new QAction(QObject::tr("Step until..."), parent_);
    cxxdbg_connect(debugStepUntilAction_, &QAction::triggered, [this] {
        dbg_.ask_line_number_and_step_until();
    });


    // Step into function menu
    debugStepIntoFunctionMenu_ = new QMenu(QObject::tr("Step into function"));

    // Step into function under cursor action
    stepIntoCurrentSymbolAction_ = new QAction(QObject::tr("Symbol: "), parent_);
    stepIntoCurrentSymbolAction_->setEnabled(false);
    cxxdbg_connect(stepIntoCurrentSymbolAction_, &QAction::triggered, [this] {
        stepIntoCurrentSymbol();
    });

    // Step into function specified by name action
    stepIntoSpecificFunctionAction_ = new QAction(QObject::tr("Specific function..."), parent_);
    cxxdbg_connect(stepIntoSpecificFunctionAction_, &QAction::triggered, [this] {
        stepIntoSpecificFunction();
    });

    debugStepIntoFunctionMenu_->addAction(stepIntoCurrentSymbolAction_);
    debugStepIntoFunctionMenu_->addSeparator();
    debugStepIntoFunctionMenu_->addAction(stepIntoSpecificFunctionAction_);


    docList_.symbol_under_cursor_changed().connect([this] {
        auto symbol = QString::fromStdWString(docList_.symbol_under_cursor());
        stepIntoCurrentSymbolAction_->setText(tr("Symbol: ") + symbol);
        bool enable = dbg_.state() == cxxdbg::dbg::debugger::state_t::stopped && !symbol.isEmpty();
        stepIntoCurrentSymbolAction_->setEnabled(enable);
    });

    dbg_.call_targets_changed().connect([this] {
        updateStepIntoMenu();
    });



    // toggle breakpoint action
    toggleBreakpointAction_ = new QAction(toggleBreakpointIcon(), tr("Toggle breakpoint"), this);
    toggleBreakpointAction_->setShortcut(QKeySequence("F9"));
    cxxdbg_connect(toggleBreakpointAction_, &QAction::triggered, [this]() {
        auto curr_doc = dynamic_cast<cxxdbg::text_file_document*>(docList_.current());
        assert(curr_doc != nullptr && "No current source");
        auto curr_line = static_cast<unsigned int>(curr_doc->current_pos().line) + 1;
        dbg_.breakpoints().toggle_breakpoint(dbg_.src_mdl().source(curr_doc->path()), curr_line);
    });

    // add breakpoint at source position action
    addSourcePosBreakpointAction_ = new QAction(addSourcePositionBreakpointIcon_,
                                                QObject::tr("Add breakpoint at position..."), parent_);
    cxxdbg_connect(addSourcePosBreakpointAction_, &QAction::triggered, [this] {
        addSourcePosBreakpoint();
    });

    // add breakpoint at function action
    addFunctionBreakpointAction_ = new QAction(addFunctionBreakpointIcon_,
                                               QObject::tr("Add breakpoint at function..."), parent_);
    cxxdbg_connect(addFunctionBreakpointAction_, &QAction::triggered, [this] {
        addFunctionBreakpoint();
    });

    // add breakpoint at address action
    addAddressBreakpointAction_ = new QAction(addAddressBreakpointIcon_,
                                              QObject::tr("Add breakpoint at address..."), parent_);
    cxxdbg_connect(addAddressBreakpointAction_, &QAction::triggered, [this] {
        addAddressBreakpoint();
    });

    // add breakpoint when exception thrown
    addExceptionThrawnBreakpointAction_ = new QAction(QObject::tr("Add breakpoint when exception thrown..."), parent_);
    cxxdbg_connect(addExceptionThrawnBreakpointAction_, &QAction::triggered, [this] {
        dbg_.breakpoints().add_breakpoint_exception_thrown([](auto && ...) {});
    });

    // add breakpoint when exception caught
    addExceptionCaughtBreakpointAction_ = new QAction(QObject::tr("Add breakpoint when exception caught..."), parent_);
    cxxdbg_connect(addExceptionCaughtBreakpointAction_, &QAction::triggered, [this] {
        dbg_.breakpoints().add_breakpoint_exception_caught([](auto && ...) {});
    });

    // add watchpoint action
    addWatchpointAction_ = new QAction(QObject::tr("Add watchpoint..."), parent_);
    cxxdbg_connect(addWatchpointAction_, &QAction::triggered, [this] {
        addWatchpoint();
    });

    // delete all breakpoints action
    deleteAllBreakpointsAction_ = new QAction(deleteAllBreakpointsIcon_,
                                              QObject::tr("Delete all breakpoints"), parent_);
    deleteAllBreakpointsAction_->setShortcut(QKeySequence("CTRL+SHIFT+F9"));
    cxxdbg_connect(deleteAllBreakpointsAction_, &QAction::triggered, [this]() {
        QMessageBox msg(QMessageBox::Question,
                        QObject::tr("Deleting all breakpoints"),
                        QObject::tr("Are you sure you want delete all breakpoints?"),
                        QMessageBox::Yes | QMessageBox::No);
        if (msg.exec() != QMessageBox::Yes)
            return;

        dbg_.breakpoints().remove_all_breakpoints();
    });

    // enable all breakpoints action
    enableAllBreakpointsAction_ = new QAction(enableAllBreakpointsIcon_,
                                              QObject::tr("Eanble all breakpoints"), parent_);
    cxxdbg_connect(enableAllBreakpointsAction_, &QAction::triggered, [this]() {
        dbg_.breakpoints().enable_all_breakpoints();
    });

    // disable all breakpoints action
    disableAllBreakpointsAction_ = new QAction(disableAllBreakpointsIcon_,
                                               QObject::tr("Disable all breakpoints"), parent_);
    cxxdbg_connect(disableAllBreakpointsAction_, &QAction::triggered, [this]() {
        dbg_.breakpoints().disable_all_breakpoints();
    });


    // show threads in source action
    showThreadsInSourceAction_ = new QAction(tr("Show threads in source"), this);
    showThreadsInSourceAction_->setCheckable(true);
    showThreadsInSourceAction_->setChecked(QSettings().value("show_threads_in_source").toBool());
}


void DebugUI::updateActionsStatus() {
    using state_t = cxxdbg::dbg::debugger::state_t;
    auto st = dbg_.state();

    // enable "continue debugging" action only in stopped state
    debugContinueAction_->setEnabled(st == state_t::stopped);

    // enable "interrtup" action only in running state
    debugInterruptAction_->setEnabled(st == state_t::running);

    // enable stepping actions only in stopped state
    debugStepIntoAction_->setEnabled(st == state_t::stopped);
    debugStepOverAction_->setEnabled(st == state_t::stopped);
    debugStepOutAction_->setEnabled(st == state_t::stopped);
    debugStepInstIntoAction_->setEnabled(st == state_t::stopped);
    debugStepInstOverAction_->setEnabled(st == state_t::stopped);
    debugStepUntilAction_->setEnabled(st == state_t::stopped);

    for (auto & action : debugStepIntoFunctionMenu_->actions()) {
        if (action == stepIntoCurrentSymbolAction_ || action->isSeparator()) {
            continue;
        }

        action->setEnabled(st == state_t::stopped);
    }


    // enable breakpoint actions in all target states
    bool enableBreakpoints = true;

    // enable toggle breakpoint only if some source is opened
    toggleBreakpointAction_->setEnabled(enableBreakpoints && docList_.current() != nullptr);

    addSourcePosBreakpointAction_->setEnabled(enableBreakpoints);
    addFunctionBreakpointAction_->setEnabled(enableBreakpoints);
    addAddressBreakpointAction_->setEnabled(enableBreakpoints);

    addExceptionThrawnBreakpointAction_->setEnabled(enableBreakpoints);
    addExceptionCaughtBreakpointAction_->setEnabled(enableBreakpoints);

    deleteAllBreakpointsAction_->setEnabled(enableBreakpoints);
    enableAllBreakpointsAction_->setEnabled(enableBreakpoints);
    disableAllBreakpointsAction_->setEnabled(enableBreakpoints);

    bool is_stopped_state = st == cxxdbg::dbg::debugger::state_t::stopped;
    addWatchpointAction_->setEnabled(is_stopped_state);
}


bool DebugUI::askStepUntilLineNumber(unsigned int & lineNum, QWidget * parent) const {
    StepUntilDialog dlg(parent);
    if (dlg.exec() == QDialog::Rejected) {
        return false;
    }

    lineNum = dlg.lineNumber();
    return true;
}


void DebugUI::initIcons() {
    QString imagesDir = QCoreApplication::applicationDirPath() + "/../share/cxxdbg/images";

    addIcon_ = makeSvgIcon("add");
    removeIcon_ = makeSvgIcon("remove");
    editIcon_ = makeSvgIcon("edit");

    debugContinueIcon_ = makeSvgIcon("resume");
    debugInterruptIcon_ = makeSvgIcon("pause");

    debugStepIntoIcon_ = makeSvgIcon("step_into");
    debugStepOverIcon_ = makeSvgIcon("step_over");
    debugStepOutIcon_ = makeSvgIcon("step_out");

    addSourcePositionBreakpointIcon_ = makeSvgIcon("break_add_source");
    addFunctionBreakpointIcon_ = makeSvgIcon("break_add_func");
    addAddressBreakpointIcon_ = makeSvgIcon("break_add_addr");

    enableBreakpointIcon_ = makeSvgIcon("break_toggle");
    disableBreakpointIcon_ = makeSvgIcon("break_disable");
    toggleBreakpointIcon_ = makeSvgIcon("break_toggle");
    breakpointIcon_ = QIcon(imagesDir + "/break.png");
    disabledBreakpointIcon_ = QIcon(imagesDir + "/disabled_break.png");

    deleteAllBreakpointsIcon_ = makeSvgIcon("break_delete_all");
    enableAllBreakpointsIcon_ = makeSvgIcon("break_enable_all");
    disableAllBreakpointsIcon_ = makeSvgIcon("break_disable_all");

    rightBlueIcon_ = QIcon(imagesDir + "/right_blue.png");
    rightYellowIcon_ = QIcon(imagesDir + "/right_yellow.png");
    threadIcon_ = QIcon(imagesDir + "/thread.png");
    switchToThreadIcon_ = QApplication::style()->standardIcon(QStyle::SP_ArrowDown);
}


void DebugUI::updateStepIntoMenu() {
    // clear actions
    auto actions = debugStepIntoFunctionMenu_->actions();
    for (auto * action : actions) {
        // don't remove step into function under cursor, step into specific function actions and separators
        // separator has type QAction
        if (action == stepIntoCurrentSymbolAction_ ||
            action == stepIntoSpecificFunctionAction_ ||
            action->isSeparator()) {

            continue;
        }

        debugStepIntoFunctionMenu_->removeAction(action);
        action->deleteLater();
    }

    // add new actions
    auto & targets = dbg_.call_targets();
    for (auto & target : targets) {
        QAction * action = new QAction(QString::fromStdString(target), debugStepIntoFunctionMenu_);
        cxxdbg_connect(action, &QAction::triggered, [this, target]() {
            dbg_.step_into_call_target(target);
        });
        debugStepIntoFunctionMenu_->addAction(action);
    }

    // updating current symbol name in menu
    auto current_symbol = docList_.symbol_under_cursor();
    auto text = tr("Symbol: ") + QString::fromStdWString(current_symbol);
    stepIntoCurrentSymbolAction_->setText(text);
}


void DebugUI::addSourcePosBreakpoint() {
    // show dialog
    EditSourcePosBreakpointDialog dlg{srcList_};
    if (dlg.exec() != QDialog::Accepted)
        return;

    // adding new breakpoint
    std::string fileName = dlg.fileName().toStdString();
    unsigned int lineNum = dlg.lineNumber();

    dbg_.breakpoints().add_breakpoint(cxxdbg::dbg::source_position_info(fs::path(fileName), lineNum),
                        [](auto && ...) {});
}


void DebugUI::addFunctionBreakpoint() {
    // show dialog
    EditFunctionBreakpointDialog dlg{dbg_};
    if (dlg.exec() != QDialog::Accepted)
        return;

    // adding new breakpoint
    dbg_.breakpoints().add_breakpoint(dlg.funcName().toStdString(), [](auto && ...) {});
}


void DebugUI::addAddressBreakpoint() {
    // show dialog
    EditAddressBreakpointDialog dlg;
    if (dlg.exec() != QDialog::Accepted)
        return;

    // adding new breakpoint
    dbg_.breakpoints().add_breakpoint(dlg.address(), [](auto && ...) {});
}


void DebugUI::addWatchpoint() {
    AddWatchpointDialog dialog(dbg_.breakpoints());
    dialog.exec();
}


void DebugUI::changeBreakpointCondition(const cxxdbg::dbg::breakpoint * bp) {

    // displaying condition dialog
    BreakpointConditionDialog dlg(mainWindow_);
    dlg.setCondition(QString::fromStdString(bp->condition()));
    if (dlg.exec() != QDialog::Accepted)
        return;

    // updating breakpoint
    dbg_.breakpoints().set_breakpoint_condition(bp, dlg.condition().toStdString());
}


void DebugUI::changeBreakpointHitCount(const cxxdbg::dbg::breakpoint * bp) {

    // displaying hit count dialog
    BreakpointHitCountDialog dlg(mainWindow_);
    dlg.setHitCountEnabled(bp->hit_count_enabled());
    dlg.setHitCount(bp->hit_count());
    dlg.setCurrHitCount(bp->curr_hit_count());
    if (dlg.exec() != QDialog::Accepted)
        return;

    // updating breakpoint
    dbg_.breakpoints().set_breakpoint_hit_count(bp,
                                                dlg.hitCountEnabled(),
                                                dlg.hitCount(),
                                                dlg.resetCurrHitCount());
}


void DebugUI::saveSettings() const {
    QSettings().setValue("show_threads_in_source", QVariant(showThreadsInSourceAction_->isChecked()));
}


QIcon DebugUI::makeSvgIcon(const QString & name) {
    auto imagesDir = QCoreApplication::applicationDirPath() + "/../share/cxxdbg/images";
    return ::cxxdbg::gui::makeSvgIcon(imagesDir, name);
}


void DebugUI::stepIntoSpecificFunction() {
    FunctionNameDialog dialog(mainWindow_);
    auto res = dialog.exec();
    if (res == QDialog::Accepted) {
        QString name = dialog.getFunctionName();
        dbg_.step_into(boost::indeterminate, name.toStdString());
    }
}


void DebugUI::stepIntoCurrentSymbol() {
    auto wsymbol = docList_.symbol_under_cursor();
    std::string symbol{wsymbol.begin(), wsymbol.end()};
    if (!symbol.empty()) {
        dbg_.step_into(boost::indeterminate, symbol);
    }
}


}
