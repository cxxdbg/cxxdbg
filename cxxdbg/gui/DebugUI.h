// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file DebugUI.h
/// Contains definition of the DebugUI class.

#pragma once

#include "cxxdbg/app/signals.hpp"
#include <QIcon>
#include <QObject>


class QAction;
class QMenu;
class QWidget;


namespace cxxdbg {
    class document_list;
    class ro_tree_view_model;
}

namespace cxxdbg::dbg {
    class breakpoint;
    class debugger;
}


namespace cxxdbg::gui {


/// Contains common code for debugger UI
class DebugUI: public QObject {
    Q_OBJECT

public:
    /// Constructs UI object with specified reference to debugger instance
    /// and document list
    DebugUI(cxxdbg::dbg::debugger & dbg,
            cxxdbg::document_list & docList,
            QObject * parent,
            const cxxdbg::ro_tree_view_model * srcList);

    /// Virtual destructor
    virtual ~DebugUI() = default;

    /// Updates debug actions status
    void updateActionsStatus();

    /// Displays step until dialog and sets lineNum to inserted value.
    /// Returns true if user clicks ok
    bool askStepUntilLineNumber(unsigned int & lineNum, QWidget * parent) const;

    QIcon addIcon() const { return addIcon_; }
    QIcon removeIcon() const { return removeIcon_; }
    QIcon editIcon() const { return editIcon_; }

    QIcon enableBreakpointIcon() const { return enableBreakpointIcon_; }
    QIcon disableBreakpointIcon() const { return disableBreakpointIcon_; }
    QIcon toggleBreakpointIcon() const { return toggleBreakpointIcon_; }
    QIcon breakpointIcon() const { return breakpointIcon_; }
    QIcon disabledBreakpointIcon() const { return disabledBreakpointIcon_; }

    QIcon rightYellowIcon() const { return rightYellowIcon_; }
    QIcon rightBlueIcon() const { return rightBlueIcon_; }
    QIcon threadIcon() const { return threadIcon_; }
    QIcon switchToThreadIcon() const { return switchToThreadIcon_; }


    QAction * debugContinueAction() { return debugContinueAction_; }
    QAction * debugInterruptAction() { return debugInterruptAction_; }
    QAction * debugStepIntoAction() { return debugStepIntoAction_; }
    QAction * debugStepOverAction() { return debugStepOverAction_; }
    QAction * debugStepOutAction() { return debugStepOutAction_; }
    QAction * debugStepInstIntoAction() { return debugStepInstIntoAction_; }
    QAction * debugStepInstOverAction() { return debugStepInstOverAction_; }
    QAction * debugStepUntilAction() { return debugStepUntilAction_; }

    QMenu * debugStepIntoFuncMenu() { return debugStepIntoFunctionMenu_; }
    QAction * stepIntoSpecificFunctionAction() { return stepIntoSpecificFunctionAction_; }
    QAction * stepIntoCurrentSymbolAction() { return stepIntoCurrentSymbolAction_; }

    QAction * addSourcePosBreakpointAction() { return addSourcePosBreakpointAction_; }
    QAction * addFunctionBreakpointAction() { return addFunctionBreakpointAction_; }
    QAction * addAddressBreakpointAction() { return addAddressBreakpointAction_; }
    QAction * addExceptionThrawnBreakpointAction() { return addExceptionThrawnBreakpointAction_; }
    QAction * addExceptionCaughtBreakpointAction() { return addExceptionCaughtBreakpointAction_; }
    QAction * addWatchpointAction() { return addWatchpointAction_; }
    QAction * toggleBreakpointAction() { return toggleBreakpointAction_; }
    QAction * deleteAllBreakpointsAction() { return deleteAllBreakpointsAction_; }
    QAction * enableAllBreakpointsAction() { return enableAllBreakpointsAction_; }
    QAction * disableAllBreakpointsAction() { return disableAllBreakpointsAction_; }

    QAction * showThreadsInSourceAction() { return showThreadsInSourceAction_; }


    /// Updates step into specific target menu
    void updateStepIntoMenu();    

    /// Displays dialog for adding source position breakpoint and adds it if user clicks ok.
    /// src_list is optional pointer to model containing all sources
    void addSourcePosBreakpoint();

    /// Displays dialog for adding function breakpoint and adds it if user clicks ok
    void addFunctionBreakpoint();

    /// Displays dialog for adding address breakpoint and adds it if user clicks ok
    void addAddressBreakpoint();

    /// Displays dialog for adding watchpoint and adds it if user clicks ok
    void addWatchpoint();

    /// Shows breakpoint condition dialog and changes condition according
    /// to entered data
    void changeBreakpointCondition(const cxxdbg::dbg::breakpoint * bp);

    /// Shows breakpoint hit count dialog and changes hit count property
    /// according to entered data
    void changeBreakpointHitCount(const cxxdbg::dbg::breakpoint * bp);

    /// Sets pointer to main window
    void setMainWindow(QWidget * mainWnd) { mainWindow_ = mainWnd; }

    /// Saves debug UI relates settings
    void saveSettings() const;

    /// Makes SVG icon with specified name
    QIcon makeSvgIcon(const QString & name);

private:
    /// Initializes common application debugging actions
    void initActions();

    /// Initializes debug icons
    void initIcons();

    /// Displays dialog for entering function name and performs step into
    /// function with entered name
    void stepIntoSpecificFunction();

    /// Detects current symbol under cursor from document list and
    /// performs step into this symbol
    void stepIntoCurrentSymbol();


    cxxdbg::dbg::debugger & dbg_;                 ///< Reference to debugger instance
    cxxdbg::document_list & docList_;             ///< Reference to document list
    QObject * parent_;                          ///< Pointer for parent object for all action
    QWidget * mainWindow_;                      ///< Pointer to parent window for all dialogs

    const cxxdbg::ro_tree_view_model * srcList_;  ///< Optional pointer to source list model

    QIcon addIcon_;
    QIcon removeIcon_;
    QIcon editIcon_;

    QIcon debugContinueIcon_;                   ///< Icon for debug continue action
    QIcon debugInterruptIcon_;                  ///< Icon for debug interrup action
    QIcon debugStepIntoIcon_;                   ///< Icon for Step Into action
    QIcon debugStepOverIcon_;                   ///< Icon for Step Over action
    QIcon debugStepOutIcon_;                    ///< Icon for Step Out action

    QIcon rightYellowIcon_;                     ///< Right yellow arrow icon
    QIcon rightBlueIcon_;                       ///< Right blue arrow icon
    QIcon threadIcon_;                          ///< Thread icon
    QIcon switchToThreadIcon_;                  ///< Switch to thread icon

    QIcon addSourcePositionBreakpointIcon_;     ///< Add source position breakpoint icon
    QIcon addFunctionBreakpointIcon_;           ///< Add function breakpoint icon
    QIcon addAddressBreakpointIcon_;            ///< ADd address breakpoint icon
    QIcon enableBreakpointIcon_;                ///< Enable breakpoint icon
    QIcon disableBreakpointIcon_;               ///< Disable breakpoint icon
    QIcon toggleBreakpointIcon_;                ///< Toggle breakpoint icon
    QIcon breakpointIcon_;                      ///< Breakpoint icon
    QIcon disabledBreakpointIcon_;              ///< Disabled breakpoint icon
    QIcon deleteAllBreakpointsIcon_;            ///< Icon for deleting all breakpoints
    QIcon enableAllBreakpointsIcon_;            ///< Icon for enabling all breakpoints
    QIcon disableAllBreakpointsIcon_;           ///< Icon for disabling all breakpoints

    QAction * debugContinueAction_;             ///< Continue action
    QAction * debugInterruptAction_;            ///< Interrupt action
    QAction * debugStepIntoAction_;             ///< Step into action
    QAction * debugStepOverAction_;             ///< Step over action
    QAction * debugStepOutAction_;              ///< Step out action
    QAction * debugStepInstIntoAction_;         ///< Step instruction into action
    QAction * debugStepInstOverAction_;         ///< Step instruction over action
    QAction * debugStepUntilAction_;            ///< Step until action

    QMenu *  debugStepIntoFunctionMenu_;        ///< Step into function menu
    QAction * stepIntoSpecificFunctionAction_;  ///< Step into function specified by name
    QAction * stepIntoCurrentSymbolAction_;     ///< Step into action under cursor action

    QAction * addSourcePosBreakpointAction_;    ///< Add breakpoint at position action
    QAction * addFunctionBreakpointAction_;     ///< Add breakpoint at function action
    QAction * addAddressBreakpointAction_;      ///< Add breakpoint at address action
    QAction * addExceptionThrawnBreakpointAction_;  ///< Add breakpoint when exception thrawn action
    QAction * addExceptionCaughtBreakpointAction_;  ///< Add breakpoint when exception caught action
    QAction * addWatchpointAction_;             ///< Add watchpoint action
    QAction * toggleBreakpointAction_;          ///< Toggle breakpoint action
    QAction * deleteAllBreakpointsAction_;      ///< Delete all breakpoints action
    QAction * enableAllBreakpointsAction_;      ///< Enable all breakpoints action
    QAction * disableAllBreakpointsAction_;     ///< Disable all breakpoints action

    QAction * showThreadsInSourceAction_;       ///< Show threads in source action

    cxxdbg::scoped_signal_connection app_state_changed_con_;
};


}
