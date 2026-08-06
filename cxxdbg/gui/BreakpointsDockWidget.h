// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file BreakpointsDockWidget.h
/// Contains definition of BreakpointsDockWidget class.

#pragma once

#include "CxxdbgDockWidget.h"
#include "CxxdbgTreeView.h"

#include <list>
#include <QKeyEvent>


class QAction;
class QTreeWidgetItem;


namespace cxxdbg::dbg {
    class breakpoints_view_model;
    class debugger;
    class source_position;
}


namespace cxxdbg::gui {

class DebugUI;


class BreakpointsDockWidget: public CxxdbgDockWidget {
    Q_OBJECT

public:
    /// Constructor, makes new breakpoints dock widget with specified
    /// reference to application object and pointer to parent widget
    explicit BreakpointsDockWidget(cxxdbg::dbg::debugger & dbg,
                                   DebugUI & dbgUi,
                                   QWidget * parent = 0);

    /// Saves widget state
    void saveState();

signals:
    /// Called when user clicks "Add breakpoint at position" action
    void addSourcePosBreakpointClicked();

    /// Called when user clicks "Add breakpoint at function" action
    void addFunctionBreakpointClicked();

    /// Called when user clicks "Go to source" action
    void goToSourceClicked(const cxxdbg::dbg::source_position &);

protected:
    /// Handles key press event
    virtual void keyPressEvent(QKeyEvent * event);

private:
    /// Called after breakpoint was added
    void onBreakpointAdded(const cxxdbg::dbg::breakpoint * bp);

    /// Called after breakpoint was removed
    void onBreakpointRemoved(const cxxdbg::dbg::breakpoint * bp);

    /// Called after breakpoint was changed
    void onBreakpointChanged(const cxxdbg::dbg::breakpoint * bp);

    /// Called when user selects copy context menu item
    void onCopy();

    /// Called when user selects delete action
    void onDelete();

    /// Called when user selects enable action
    void onEnable();

    /// Called when user selects disable action
    void onDisable();

    /// Called when user selects go to source action
    void onGoToSource();

    /// Called when user selects condition action
    void onCondition();

    /// Called when user selects hit count action
    void onHitCount();

    /// Updates breakpoint tree item
    void updateBreakpointItem(QTreeWidgetItem * item, const cxxdbg::dbg::code_breakpoint * bp);

    /// Called after breakpoint selection was changed
    void onSelectionChanged();

    /// Makes new tree widget item for breakpoint location
    QTreeWidgetItem * makeBreakpointLocationItem(const cxxdbg::dbg::breakpoint_location * loc);

    /// Finds breakpoint location item for specified breakpoint item and breakpoint location
    QTreeWidgetItem * findBreakpointLocationItem(QTreeWidgetItem * bpItem,
                                                 const cxxdbg::dbg::breakpoint_location * loc);

    /// Returns list of selected breakpoints
    std::list<const cxxdbg::dbg::breakpoint*> selectedBreakpoints();

    /// Returns selected breakpoint
    const cxxdbg::dbg::breakpoint * selectedBreakpoint();

    /// Returns selected breakpoint location if selected single location item
    /// or breakpoint item containing single location. Else returns nullptr
    const cxxdbg::dbg::breakpoint_location * selectedBreakpointLocation();

    const cxxdbg::dbg::source_position selectedItemPosition();


    cxxdbg::dbg::debugger & dbg_;                         ///< Reference to debugger object
    DebugUI & dbgUi_;                                   ///< Reference to common debug UI
    cxxdbg::dbg::breakpoints_view_model & model_;         ///< Breakpoints view model
    CxxdbgTreeView * breakpoints_;                        ///< Breakpoints and locations tree
    AppRoItemModel::ImageConverterSP iconConverter_;    ///< Image converter interface implementation
    std::unique_ptr<AppRoItemModel> appModel_;          ///< Qt model for breakpoints view

    QAction * copyAction_;              ///< Copy content action
    QAction * selectAllAction_;         ///< Select all breakpoints action
    QAction * deleteAction_;            ///< Delete breakpoint(s) action
    QAction * enableAction_;            ///< Enable breakpoint(s) action
    QAction * disableAction_;           ///< Disable breakpoint(s) action
    QAction * goToSourceAction_;        ///< Go to source action
    QAction * conditionAction_;         ///< Breakpoint condition action
    QAction * hitCountAction_;          ///< Breakpoint hit count action


    /// Connection to application state changed signal
    cxxdbg::scoped_signal_connection appStateChangedCon_;

    /// Connection to breakpoint added signal
    cxxdbg::scoped_signal_connection breakpointAddedCon_;

    /// Connection to breakpoint removed signal
    cxxdbg::scoped_signal_connection breakpointRemovedCon_;

    /// Connection to breakpoint changed signal
    cxxdbg::scoped_signal_connection breakpointChangedCon_;

    /// Connection to breakpoint location added signal
    cxxdbg::scoped_signal_connection breakpointLocationAddedCon_;

    /// Connection to breakpoint location removed signal
    cxxdbg::scoped_signal_connection breakpointLocationRemovedCon_;

    /// Connection to breakpoint location changed signal
    cxxdbg::scoped_signal_connection breakpointLocationChangedCon_;
};


}
