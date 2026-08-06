// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file SourceTreeDockWidget
/// Contains implementation of SourceTreeDockWidget class.

#include "SourceTreeDockWidget.h"
#include "CxxdbgApplication.h"
#include "CxxdbgTreeView.h"
#include "cxxdbg_connect.hpp"
#include "cxxdbg/dbg/composite_source_tree_model.hpp"
#include "cxxdbg/dbg/debugger.hpp"
#include "cxxdbg/dbg/source_file.hpp"
#include "cxxdbg/dbg/source_tree.hpp"
#include "cxxdbg/dbg/target.hpp"

#include <filesystem>
#include <iostream>
#include <map>
#include <set>

#include <QAction>
#include <QFileIconProvider>
#include <QHeaderView>
#include <QMessageBox>
#include <QTreeWidget>
#include <QTreeView>
#include <QToolBar>
#include <QStandardItem>
#include <QUuid>
#include <QIcon>
#include <QDebug>
#include <QHash>
#include <QVector>


namespace fs = std::filesystem;


namespace cxxdbg::gui {


class ImageConverter: public AppRoItemModel::ImageConverter {
public:
    explicit ImageConverter(QStyle * style):
        style_{style} {}

    /// Converts image index to icon
    QIcon get(size_t idx, const cxxdbg::tree_view_model::row_index &) const override {
        if (idx == cxxdbg::dbg::composite_source_tree_model::image_index_directory) {
            return style_->standardIcon(QStyle::SP_DirIcon);
        }

        return style_->standardIcon(QStyle::SP_FileIcon);
    }

private:
    QStyle * style_;
};


SourceTreeDockWidget::SourceTreeDockWidget(CxxdbgApplication & app) :
CxxdbgDockWidget(tr("Source tree")),
app_(app),
image_converter_(new ImageConverter(style())),
model_(app.trees_model()),
qtModel_(model_, image_converter_) {
    setObjectName("source_tree");

    // open action
    QIcon openIcon = app_.dbgUi().makeSvgIcon("open");
    openSourceAction_ = new QAction{openIcon, tr("Open"), this};
    cxxdbg_connect(openSourceAction_, &QAction::triggered, [this]() {
        // get current index
        QModelIndex index = tree_->currentIndex();
        assert(index.isValid() && "Current item is not valid");
        auto tree_item = get_tree_item(index);
        assert(tree_item != nullptr && "Current item is not a tree item");
        assert(tree_item->is_file() && "Current item doesn't point to a file");

        // open file
        app_.documents().open_text<true>(tree_item->path(), false);
    });

    tree_ = new CxxdbgTreeView("source_tree", this);
    setWidget(tree_);
    tree_->setModel(&qtModel_);
    tree_->setFrameStyle(QFrame::NoFrame);
    tree_->header()->hide();

    cxxdbg_connect(tree_, &QTreeView::expanded,
            [this]() { tree_->resizeColumnToContents(0); });

    cxxdbg_connect(tree_->selectionModel(), &QItemSelectionModel::currentChanged,
    [this](const QModelIndex & current, const QModelIndex & previous) {
        onSelectionChanged(current, previous);
    });
    tree_->addAction(openSourceAction_);
    tree_->setContextMenuPolicy(Qt::ActionsContextMenu);

    // handle double click for source tree
    cxxdbg_connect(tree_, &QTreeView::doubleClicked, [this](const QModelIndex & index) {
        // trigger open file action if it is enabled
        if (openSourceAction_->isEnabled())
                emit openSourceAction_->triggered();
    });

    // handle sources changed signal
    sources_changed_con_ = app_.connect_sources_changed([this] { onSourcesChanged(); });

    // displaying main source when target changes
    target_changed_con_ = app_.dbg().target_changed().connect([this](auto && res, auto && mods, auto && mpos) {
        if (app_.has_target()) {
            // loaded new target

            if (!mpos) {
                return;
            }

            auto row_idx = model_.path_index(mpos.file()->path());
            if (!row_idx.is_valid()) {
                return;
            }

            auto mdl_idx = qtModel_.convertIndex(row_idx);
            tree_->scrollTo(mdl_idx);
            tree_->selectionModel()->setCurrentIndex(mdl_idx, QItemSelectionModel::Select);
        }
    });

    onSelectionChanged(QModelIndex(), QModelIndex());
    onSourcesChanged();
}


void SourceTreeDockWidget::onSelectionChanged(const QModelIndex & current, const QModelIndex & previous) {
    // disable open action for non file item
    openSourceAction_->setEnabled(current.isValid() && isExistingFileItem(current));
}


void SourceTreeDockWidget::onSourcesChanged() {
    // remove all items
    //sourceTree_->clear();

    // check if target is loaded
    if (!app_.has_target()) {
        tree_->setEnabled(false);
        return;
    }

    tree_->setEnabled(true);
}


bool SourceTreeDockWidget::isExistingFileItem(const QModelIndex &index) {
    // get tree item
    auto tree_item = get_tree_item(index);

    if (tree_item != nullptr) {
        if (tree_item->is_file()) {
            return fs::exists(tree_item->path());
        }
    }

    return false;
}

SourceTreeDockWidget::~SourceTreeDockWidget() {

}

const cxxdbg::dbg::source_tree_item * SourceTreeDockWidget::get_tree_item(const QModelIndex & index) {
    assert(index.isValid() && "invalid tree index");
    auto row_index = qtModel_.appRow(index);
    return model_.row_to_node(row_index);
}


}
