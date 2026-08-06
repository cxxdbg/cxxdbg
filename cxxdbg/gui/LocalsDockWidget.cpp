// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file WatchListDockWidget.cpp
/// Contains implementation of WatchListDockWidget class.

#include "LocalsDockWidget.h"
#include "WatchListTreeView.h"
#include "WatchListColorConverter.h"
#include "Utils.h"
#include "cxxdbg_connect.hpp"

#include "cxxdbg/app/tree_view_model.hpp"
#include "cxxdbg/dbg/debugger.hpp"
#include "cxxdbg/dbg/watch_list.hpp"
#include "cxxdbg/dbg/watch_model.hpp"

#include <QToolBar>
#include <QCoreApplication>
#include <QtWidgets/QtWidgets>
#include <QAbstractItemView>


namespace cxxdbg::gui {


LocalsDockWidget::LocalsDockWidget(cxxdbg::dbg::debugger & dbg, document_navigator & doc_nav):
CxxdbgDockWidget{tr("Locals")},
locModel_{dbg.locals()},
sortableModel_{locModel_},
locQtModel_{sortableModel_}
{

    setObjectName("locals");

    locQtModel_.setColorConverter(WatchListColorConverter::createInstance());

    // creating watch tree
    tree_ = new WatchListTreeView{"ui/locals/tree", dbg, locQtModel_};
    tree_->setFrameStyle(QFrame::NoFrame);
    setWidget(tree_);

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
    toolBar()->addAction(goto_definition_action_);
    toolBar()->addAction(goto_value_definition_action_);
    toolBar()->addAction(goto_typedef_action_);

    // connect actions
    cxxdbg_connect(goto_definition_action_, &QAction::triggered, [this, &doc_nav]() {
        auto * node = getSelectedNode();  // cxxdbg::app::watch_list_tree_node
        if (nullptr == node)
            return;

        auto & pos = node->pos();
        assert(pos.is_valid() && "pos must be valid whe action is enabled");
        doc_nav.show_pos(pos.path(), static_cast<size_t>(pos.line() - 1));
    });

    cxxdbg_connect(goto_value_definition_action_, &QAction::triggered, [this, &doc_nav]() {
        auto * node = getSelectedNode();
        if (node == nullptr)
            return;

        auto & pos = node->val_pos();
        assert(pos.is_valid() && "pos must be valid whe action is enabled");
        doc_nav.show_pos(pos.path(), static_cast<size_t>(pos.line() - 1));
    });

    cxxdbg_connect(goto_typedef_action_, &QAction::triggered, [this, &doc_nav]() {
        auto * node = getSelectedNode();
        if (node == nullptr)
            return;

        auto & pos = node->type_pos();
        assert(pos.is_valid() && "pos must be valid whe action is enabled");
        doc_nav.show_pos(pos.path(), static_cast<size_t>(pos.line() - 1));
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

    // update state
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


LocalsDockWidget::~LocalsDockWidget() {
}


void LocalsDockWidget::saveState() {
    tree_->saveState();
}


cxxdbg::dbg::watch_list_tree_node * LocalsDockWidget::getSelectedNode() {
    QModelIndexList sel = tree_->selectionModel()->selectedRows();

    assert(sel.count() == 1 && "only one row must be selected");
    QModelIndex qtIndex = sel.front();
    auto locIndex = locModel_.make_index(qtIndex.internalPointer());

    if (locIndex.is_valid()) {
        return reinterpret_cast<cxxdbg::dbg::watch_list_tree_node *>(locIndex.ptr());
    }

    return nullptr;
}


}
