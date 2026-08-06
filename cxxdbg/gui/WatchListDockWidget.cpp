// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file WatchListDockWidget.cpp
/// Contains implementation of the WatchListDockWdiget class.

#include "WatchListDockWidget.h"
#include "AddWatchDialog.h"
#include "WatchListTreeView.h"
#include "WatchListColorConverter.h"
#include "CxxdbgApplication.h"
#include "Utils.h"
#include "cxxdbg_connect.hpp"

#include "cxxdbg/dbg/custom_watch_list.hpp"
#include "cxxdbg/dbg/debugger.hpp"

#include <QAction>
#include <QToolBar>
#include <QKeyEvent>
#include <QCoreApplication>
#include <QDebug>


namespace cxxdbg::gui {


WatchListDockWidget::WatchListDockWidget(cxxdbg::dbg::debugger & dbg,
                                         document_navigator & doc_nav,
                                         DebugUI & dbgUi):
CxxdbgDockWidget{tr("Watch")},
model_{dbg.watch()},
sortableModel_{model_},
qtModel_{sortableModel_}
{
    setObjectName("watch");

    qtModel_.setColorConverter(WatchListColorConverter::createInstance());

    addAction_ = new QAction{dbgUi.addIcon(), tr("Add"), this};
    cxxdbg_connect(addAction_, &QAction::triggered, [this] { onAddClicked(); });

    deleteAction_ = new QAction(dbgUi.removeIcon(), tr("Delete"), this);
    
    deleteAction_->setEnabled(false);
    cxxdbg_connect(deleteAction_, &QAction::triggered, [this] { onDeleteClicked(); });

    editAction_ = new QAction(dbgUi.editIcon(), tr("Edit"), this);
    editAction_->setEnabled(false);
    cxxdbg_connect(editAction_, &QAction::triggered, [this] { onEditClicked(); });

    toolBar()->addAction(addAction_);
    toolBar()->addAction(deleteAction_);
    toolBar()->addAction(editAction_);

    tree_ = new WatchListTreeView{"ui/watch/tree", dbg, qtModel_};
    setWidget(tree_);
    tree_->setFrameStyle(QFrame::NoFrame);

    // additional context menu for tree widget
    tree_->addSeparator();

    tree_->addAction(addAction_);
    tree_->addAction(deleteAction_);
    tree_->addAction(editAction_);

    // initialize icons
    QString imagesDir = QCoreApplication::applicationDirPath() + "/../share/cxxdbg/images/";
    goto_definition_icon_.reset(new QIcon(makeSvgIcon(imagesDir, "goto_def")));
    goto_value_definition_icon_.reset(new QIcon(makeSvgIcon(imagesDir, "goto_val")));
    goto_typedef_icon_.reset(new QIcon(makeSvgIcon(imagesDir, "goto_type")));

    // initialize actions
    goto_definition_action_ = new QAction{*goto_definition_icon_, tr("Go to definition"), this};
    goto_value_definition_action_ = new QAction{*goto_value_definition_icon_, tr("Go to value definition"), this};
    goto_typedef_action_ = new QAction{*goto_typedef_icon_, tr("Go to type definition"), this};

    // add actions to tree
    tree_->addSeparator();
    tree_->addAction(goto_definition_action_);
    tree_->addAction(goto_value_definition_action_);
    tree_->addAction(goto_typedef_action_);

    // add actions to toolbar
    toolBar()->addSeparator();
    toolBar()->addAction(goto_definition_action_);
    toolBar()->addAction(goto_value_definition_action_);
    toolBar()->addAction(goto_typedef_action_);

    // connect actions
    cxxdbg_connect(goto_definition_action_, &QAction::triggered, [this, &doc_nav]() {
        auto * node = getSelectedNode();  // cxxdbg::dbg::watch_list_tree_node
        if (nullptr == node) {
            return;
        }

        auto & pos = node->pos();
        assert(pos.is_valid() && "pos must be valid whe action is enabled");
        doc_nav.show_pos(pos.path(), static_cast<unsigned int>(pos.line() - 1));
    });

    cxxdbg_connect(goto_value_definition_action_, &QAction::triggered, [this, &doc_nav]() {
        auto * node = getSelectedNode();
        if (node == nullptr) {
            return;
        }

        auto & pos = node->val_pos();
        assert(pos.is_valid() && "pos must be valid whe action is enabled");
        doc_nav.show_pos(pos.path(), static_cast<unsigned int>(pos.line() - 1));
    });

    cxxdbg_connect(goto_typedef_action_, &QAction::triggered, [this, &doc_nav]() {
        auto * node = getSelectedNode();
        if (node == nullptr) {
            return;
        }

        auto & pos = node->type_pos();
        assert(pos.is_valid() && "pos must be valid whe action is enabled");
        doc_nav.show_pos(pos.path(), static_cast<unsigned int>(pos.line() - 1));
    });

    // update actions availability
    auto updateAvailability = [this] () {
        auto selection = this->tree_->selectionModel()->selectedRows();
        bool enabled = selection.count() == 1;
        if (enabled) {
            auto * node = getSelectedNode();
            this->goto_definition_action_->setEnabled(node != nullptr && node->pos().is_valid());
            this->goto_value_definition_action_->setEnabled(node != nullptr && node->val_pos().is_valid());
            this->goto_typedef_action_->setEnabled(node != nullptr && node->type_pos().is_valid());
        } else {
            this->goto_definition_action_->setEnabled(false);
            this->goto_value_definition_action_->setEnabled(false);
            this->goto_typedef_action_->setEnabled(false);
        }
    };

    cxxdbg_connect(tree_->selectionModel(), &QItemSelectionModel::selectionChanged, updateAvailability);

    updateAvailability();

    // enable edit / delete actions only if root row is selected in tree view
    cxxdbg_connect(tree_->selectionModel(), &QItemSelectionModel::selectionChanged,
    [this] {
        auto sel = selectedRootRows();
        this->deleteAction_->setEnabled(!sel.empty());
        this->editAction_->setEnabled(sel.size() == 1);
    });

    // enable widget only in initial / loaded / stopped state
    auto handleState = [this, &dbg, updateAvailability] {
        setContentEnabled(dbg.state() == cxxdbg::dbg::debugger::state_t::initial ||
                          dbg.state() == cxxdbg::dbg::debugger::state_t::loaded ||
                          dbg.state() == cxxdbg::dbg::debugger::state_t::stopped);
        updateAvailability();
    };
    handleState();
    state_changed_con_ = dbg.state_changed().connect(handleState);
}


WatchListDockWidget::~WatchListDockWidget() {
}


void WatchListDockWidget::saveState() {
    tree_->saveState();
}


void WatchListDockWidget::keyPressEvent(QKeyEvent * event) {

    // delete current breakpoint if selected
    if (event->key() == Qt::Key_Delete) {
        if (deleteAction_->isEnabled())
            deleteAction_->trigger();
    }

    QDockWidget::keyPressEvent(event);
}


void WatchListDockWidget::onAddClicked() {
    AddWatchDialog dlg{this};
    if (dlg.exec() != QDialog::Accepted)
        return;

    model_.add(dlg.watchExpr().toStdWString());
    assert(model_.childs_size() != 0);
}


void WatchListDockWidget::onDeleteClicked() {
    auto sel = selectedRootRows();
    while (!sel.empty()) {
        model_.remove(sel.back().row());
        sel.pop_back();
    }
}


void WatchListDockWidget::onEditClicked() {
    // opening editor for selected item
    auto sel = selectedRootRows();
    assert(sel.size() == 1 && "single row should be selected");
    tree_->edit(sel.front());
}


std::list<QModelIndex> WatchListDockWidget::selectedRootRows() {
    std::list<QModelIndex> res;
    QModelIndexList sel = tree_->selectionModel()->selectedRows();
    for (auto idx : sel) {
        if (!idx.parent().isValid())
            res.push_back(idx);
    }

    return res;
}

cxxdbg::dbg::watch_list_tree_node * WatchListDockWidget::getSelectedNode() {
    QModelIndexList sel = tree_->selectionModel()->selectedRows();

    assert(sel.count() == 1 && "only one row must be selected");
    QModelIndex qtIndex = sel.front();
    auto locIndex = sortableModel_.make_index(qtIndex.internalPointer()); // cxxdbg::dbg::ro_tree_view_model::row_index

    if (locIndex.is_valid()) {
        return reinterpret_cast<cxxdbg::dbg::watch_list_tree_node *>(locIndex.ptr());
    }

    return nullptr;
}


}
