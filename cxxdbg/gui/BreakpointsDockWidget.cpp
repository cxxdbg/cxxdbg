// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file BreakpointsDockWidget.cpp
/// Contains implementation of BreakpointsDockWidget class.

#include "BreakpointsDockWidget.h"
#include "BreakpointConditionDialog.h"
#include "BreakpointHitCountDialog.h"
#include "DebugUI.h"
#include "CxxdbgTreeView.h"
#include "cxxdbg_connect.hpp"
#include "cxxdbg/dbg/breakpoint.hpp"
#include "cxxdbg/dbg/breakpoint_list.hpp"
#include "cxxdbg/dbg/breakpoint_location.hpp"
#include "cxxdbg/dbg/breakpoints_view_model.hpp"
#include "cxxdbg/dbg/code_breakpoint.hpp"
#include "cxxdbg/dbg/debugger.hpp"
#include <sstream>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMessageBox>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QDebug>


Q_DECLARE_METATYPE(const cxxdbg::dbg::code_breakpoint*)
Q_DECLARE_METATYPE(const cxxdbg::dbg::breakpoint_location*)


namespace cxxdbg::gui {


///// Searches for breakpoint item with specified breakpoint. Widget item should exist

namespace breakpoints {
class IconConverter: public AppRoItemModel::ImageConverter {
public:
    /// Virtual destructor
    virtual ~IconConverter() {}

    /// Constructor, takes two types of icons
    IconConverter(const QIcon & enabledIcon, const QIcon & disabledIcon):
            enabledIcon_(enabledIcon), disabledIcon_(disabledIcon) {
    }

    /// Converts image index to icon
    QIcon get(size_t idx, const cxxdbg::ro_tree_view_model::row_index &) const override {
        switch (idx) {
            case 1:
                return enabledIcon_;
            case 2:
                return disabledIcon_;
            default:
                return {};
        }
    }

private:
    QIcon enabledIcon_;
    QIcon disabledIcon_;
};
}


BreakpointsDockWidget::BreakpointsDockWidget(cxxdbg::dbg::debugger & dbg,
                                             DebugUI & dbgUi,
                                             QWidget * parent):
CxxdbgDockWidget(tr("Breakpoints"), parent),
dbg_(dbg),
dbgUi_(dbgUi),
model_(dbg_.breakpoints_model()) {
    setObjectName("breakpoints");

    iconConverter_ = std::make_shared<breakpoints::IconConverter>(dbgUi_.breakpointIcon(),
                                                                  dbgUi_.disabledBreakpointIcon());
    appModel_ = std::make_unique<AppRoItemModel>(model_, iconConverter_);

    // copy action
    copyAction_ = new QAction(QIcon::fromTheme("edit-copy"), tr("Copy"), this);
    cxxdbg_connect(copyAction_, &QAction::triggered, [this] { onCopy(); });

    // select all action
    selectAllAction_ = new QAction(QIcon::fromTheme("edit-select-all"), tr("Select all"), this);
    cxxdbg_connect(selectAllAction_, &QAction::triggered, [this]() { breakpoints_->selectAll(); });

    // delete action
    deleteAction_ = new QAction(dbgUi_.removeIcon(), tr("Delete"), this);
    cxxdbg_connect(deleteAction_, &QAction::triggered, [this] { onDelete(); });

    // enable action
    enableAction_ = new QAction(dbgUi_.enableBreakpointIcon(), tr("Enable"), this);
    cxxdbg_connect(enableAction_, &QAction::triggered, [this] { onEnable(); });

    // disable action
    disableAction_ = new QAction(dbgUi_.disableBreakpointIcon(), tr("Disable"), this);
    cxxdbg_connect(disableAction_, &QAction::triggered, [this] { onDisable(); });

    // go to source action
    goToSourceAction_ = new QAction(dbgUi_.makeSvgIcon("goto"), tr("Go to source"), this);
    cxxdbg_connect(goToSourceAction_, &QAction::triggered, [this] { onGoToSource(); });

    // condition action
    conditionAction_ = new QAction(tr("Condition..."), this);
    cxxdbg_connect(conditionAction_, &QAction::triggered, [this] { onCondition(); });

    // hit count action
    hitCountAction_ = new QAction(tr("Hit count..."), this);
    cxxdbg_connect(hitCountAction_, &QAction::triggered, [this] { onHitCount(); });


    // toolbar

    toolBar()->addAction(dbgUi_.addSourcePosBreakpointAction());
    toolBar()->addAction(dbgUi_.addFunctionBreakpointAction());
    toolBar()->addAction(dbgUi_.addAddressBreakpointAction());
    toolBar()->addSeparator();
    toolBar()->addAction(deleteAction_);
    toolBar()->addAction(enableAction_);
    toolBar()->addAction(disableAction_);
    toolBar()->addSeparator();
    toolBar()->addAction(dbgUi_.deleteAllBreakpointsAction());
    toolBar()->addAction(dbgUi_.enableAllBreakpointsAction());
    toolBar()->addAction(dbgUi_.disableAllBreakpointsAction());
    toolBar()->addSeparator();
    toolBar()->addAction(goToSourceAction_);

    // breakpoint tree view
    breakpoints_ = new CxxdbgTreeView("ui/breakpoints/tree");
    setWidget(breakpoints_);
    breakpoints_->setFrameStyle(QFrame::NoFrame);
    breakpoints_->setExpandsOnDoubleClick(true);
    breakpoints_->setSelectionMode(QAbstractItemView::ContiguousSelection);

    // go to source on double click
    cxxdbg_connect(breakpoints_, &CxxdbgTreeView::onDoubleClicked, [this]() {
        if (goToSourceAction_->isEnabled())
            goToSourceAction_->trigger();
    });

    // context menu
    breakpoints_->setContextMenuPolicy(Qt::ActionsContextMenu);

    breakpoints_->addSeparator();

    breakpoints_->addAction(deleteAction_);
    breakpoints_->addAction(enableAction_);
    breakpoints_->addAction(disableAction_);

    breakpoints_->addSeparator();

    breakpoints_->addAction(conditionAction_);
    breakpoints_->addAction(hitCountAction_);

    breakpoints_->addSeparator();

    breakpoints_->addAction(goToSourceAction_);

    breakpoints_->addSeparator();

    breakpoints_->addAction(dbgUi_.deleteAllBreakpointsAction());
    breakpoints_->addAction(dbgUi_.enableAllBreakpointsAction());
    breakpoints_->addAction(dbgUi_.disableAllBreakpointsAction());

    breakpoints_->addSeparator();

    breakpoints_->addAction(dbgUi_.addSourcePosBreakpointAction());
    breakpoints_->addAction(dbgUi_.addFunctionBreakpointAction());
    breakpoints_->addAction(dbgUi_.addAddressBreakpointAction());

    breakpoints_->addSeparator();

    breakpoints_->addAction(dbgUi_.addExceptionThrawnBreakpointAction());
    breakpoints_->addAction(dbgUi_.addExceptionCaughtBreakpointAction());

    breakpoints_->addSeparator();

    breakpoints_->addAction(dbgUi_.addWatchpointAction());

    breakpoints_->setModel(appModel_.get());

    breakpoints_->restoreState();

    cxxdbg_connect(breakpoints_->selectionModel(), &QItemSelectionModel::selectionChanged, [this] { onSelectionChanged(); });

    dbg_.breakpoints().list_changed().connect([this](){
        this->onSelectionChanged();
    });

    // update selection
    onSelectionChanged();
}


void BreakpointsDockWidget::saveState() {
    breakpoints_->saveState();
}


void BreakpointsDockWidget::keyPressEvent(QKeyEvent * event) {

    // delete current breakpoint if selected
    if (event->key() == Qt::Key_Delete) {
        if (deleteAction_->isEnabled())
            deleteAction_->trigger();
    }

    QDockWidget::keyPressEvent(event);
}


void BreakpointsDockWidget::onDelete() {

    // removing all selected breakpoints
    std::list<const cxxdbg::dbg::breakpoint*> bps = selectedBreakpoints();
    for (auto it = bps.begin(), end = bps.end(); it != end; ++it) {
        dbg_.breakpoints().remove_breakpoint(*it);
    }
}


void BreakpointsDockWidget::onEnable() {

    // enabling all selected breakpoints
    std::list<const cxxdbg::dbg::breakpoint*> bps = selectedBreakpoints();
    for (auto it = bps.begin(), end = bps.end(); it != end; ++it) {
        dbg_.breakpoints().enable_breakpoint(*it);
    }
}


void BreakpointsDockWidget::onDisable() {

    // disabling all selected breakpoints
    std::list<const cxxdbg::dbg::breakpoint*> bps = selectedBreakpoints();
    for (auto it = bps.begin(), end = bps.end(); it != end; ++it) {
        dbg_.breakpoints().disable_breakpoint(*it);
    }
}


void BreakpointsDockWidget::onGoToSource() {
    auto pos = selectedItemPosition();
    assert(pos.is_valid() && "breakpoint location does not have source position info");
    goToSourceClicked(pos);
}


void BreakpointsDockWidget::onCondition() {
    auto * breakpoint = selectedBreakpoint();
    dbgUi_.changeBreakpointCondition(breakpoint);
}


void BreakpointsDockWidget::onHitCount() {
    auto * breakpoint = selectedBreakpoint();
    dbgUi_.changeBreakpointHitCount(breakpoint);
}


void BreakpointsDockWidget::onCopy() {
//    std::ostringstream str;
//
//    for (int bpIndex = 0, bpCount = breakpoints_->topLevelItemCount(); bpIndex < bpCount; ++bpIndex) {
//        QTreeWidgetItem * bpItem = breakpoints_->topLevelItem(bpIndex);
//
//        str << bpItem->text(0).toStdString() << "\t\t"
//            << bpItem->text(1).toStdString() << "\t"
//            << bpItem->text(2).toStdString() << "\t"
//            << bpItem->text(3).toStdString() << "\t"
//            << bpItem->text(4).toStdString() << "\n";
//
//        for (int locIndex = 0, locCount = bpItem->childCount(); locIndex < locCount; ++locIndex) {
//            QTreeWidgetItem * locItem = bpItem->child(locIndex);
//
//            str << "\t"
//                << locItem->text(0).toStdString() << "\t"
//                << locItem->text(1).toStdString() << "\t"
//                << locItem->text(2).toStdString() << "\t"
//                << locItem->text(3).toStdString() << "\t"
//                << locItem->text(4).toStdString() << "\n";
//        }
//    }
//
//    QApplication::clipboard()->setText(QString::fromStdString(str.str()));
}

// enable or disable actions depending on selection
void BreakpointsDockWidget::onSelectionChanged() {
    std::list<const cxxdbg::dbg::breakpoint*> selBps = selectedBreakpoints();

    bool selected = !selBps.empty();
    bool selectedSingle = selBps.size() == 1;

    // enable copy and delete actions only if any breakpoint is selected
    copyAction_->setEnabled(selected);
    deleteAction_->setEnabled(selected);

    // enable breakpoint properties actions only if selected single breakpoint
    conditionAction_->setEnabled(selectedSingle);
    hitCountAction_->setEnabled(selectedSingle);

    // enable 'enable' action only if some selected breakpoint is disabled
    {
        bool found = false;
        for (auto it = selBps.begin(), end = selBps.end(); it != end; ++it) {
            if (!(*it)->enabled()) {
                found = true;
                break;
            }
        }

        enableAction_->setEnabled(found);
    }

    // enable 'disable' action only if some selected breakpoint is enabled
    {
        bool found = false;
        for (auto it = selBps.begin(), end = selBps.end(); it != end; ++it) {
            if ((*it)->enabled()) {
                found = true;
                break;
            }
        }

        disableAction_->setEnabled(found);
    }

    // enable go to source action only if selected location item with available source position
    auto pos = selectedItemPosition();
    goToSourceAction_->setEnabled(pos.is_valid() && std::filesystem::exists(pos.file()->path()));
}


std::list<const cxxdbg::dbg::breakpoint*> BreakpointsDockWidget::selectedBreakpoints() {

    QList<QModelIndex> indexes = breakpoints_->selectionModel()->selectedRows();
    std::list<const cxxdbg::dbg::breakpoint *> bps;

    for (QModelIndex index : indexes) {
        auto tree_index = appModel_->appRow(index);
        assert(tree_index.is_valid() && "no data in node");

        auto * bp = model_.get_breakpoint(tree_index);

        if (bp != nullptr) {
            bps.push_back(bp);
        }
    }

    return bps;
}


const cxxdbg::dbg::breakpoint * BreakpointsDockWidget::selectedBreakpoint() {
    std::list<const cxxdbg::dbg::breakpoint*> bps = selectedBreakpoints();
    assert(bps.size() == 1 && "No single selected breakpoint");
    return bps.front();
}


const cxxdbg::dbg::breakpoint_location * BreakpointsDockWidget::selectedBreakpointLocation() {

    QList<QModelIndex> indexes = breakpoints_->selectionModel()->selectedRows();

    if (indexes.size() != 1) {
        // "selected zero or more than one item"
        return nullptr;
    }

    auto tree_index = appModel_->appRow(indexes.front());

    auto * location = model_.get_single_location(tree_index);

    return location;
}

const cxxdbg::dbg::source_position BreakpointsDockWidget::selectedItemPosition() {
    QList<QModelIndex> indexes = breakpoints_->selectionModel()->selectedRows();

    if (indexes.size() != 1) {
        // "selected zero or more than one item"
        return {};
    }

    auto tree_index = appModel_->appRow(indexes.front());

    auto * site = model_.get_breakpoint_site(tree_index);
    assert(site && "can't find breakpoint site for index");
    return site->get_source_position();
}


}
