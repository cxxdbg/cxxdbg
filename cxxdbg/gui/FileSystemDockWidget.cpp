// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file FileSystemDockWidget.cpp
/// Contains implementation of FileSystemDockWidget class.

#include "FileSystemDockWidget.h"
#include "cxxdbg_connect.hpp"
#include <cassert>
#include <QAction>
#include <QDir>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QIcon>
#include <QMessageBox>
#include <QStyle>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>


namespace cxxdbg::gui {


FileSystemDockWidget::FileSystemDockWidget(QWidget * parent) :
CxxdbgDockWidget(tr("Filesystem"), parent) {
    setObjectName("filesystem");

    // open file action
    QIcon openIcon = style()->standardIcon(QStyle::SP_DirOpenIcon);
    openFileAction_ = new QAction(openIcon, tr("Open"), this);

    // body widget
    QWidget * body = new QWidget;
    setWidget(body);
    QVBoxLayout * layout = new QVBoxLayout(body);
    layout->setSpacing(0);
    layout->setContentsMargins({});

    // file system tree
    QFileSystemModel * model = new QFileSystemModel(this);
    model->setRootPath("/");
    QTreeView * tree = new QTreeView;
    layout->addWidget(tree);
    tree->setModel(model);
    tree->setFrameStyle(QFrame::NoFrame);
    tree->hideColumn(1);
    tree->hideColumn(2);
    tree->hideColumn(3);
    tree->header()->hide();
    tree->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QItemSelectionModel * selModel = new QItemSelectionModel(model, this);
    tree->setSelectionModel(selModel);

    // triggering open file action on double click on file
    cxxdbg_connect(tree, &QTreeView::doubleClicked, [this](auto index) {
        if (openFileAction_->isEnabled()) {
            openFileAction_->trigger();
        }
    });

    // open button handler
    cxxdbg_connect(openFileAction_, &QAction::triggered, [this, model, tree](auto && ...) {
        // get selected file
        QModelIndex index = tree->currentIndex();
        assert(index.isValid() && "Model index is not valid");
        QFileInfo info = model->fileInfo(index);

        // check that file is not directory
        assert(!info.isDir() && "File is directory");

        emit openSelected(info.filePath());
    });


    // update menu status on selection change
    cxxdbg_connect(selModel, &QItemSelectionModel::currentChanged,
            [this, model](const QModelIndex & cur, const QModelIndex &) {
        bool openEnabled = false;

        if (cur.isValid()) {
            QFileInfo info = model->fileInfo(cur);
            if (!info.isDir())
                openEnabled = true;
        }

        openFileAction_->setEnabled(openEnabled);
    });


    // context menu
    tree->addAction(openFileAction_);
    tree->setContextMenuPolicy(Qt::ActionsContextMenu);


    // display home
    QModelIndex homeIndex = model->index(QDir::homePath());
    QModelIndex index = homeIndex;
    while (index.isValid()) {
        tree->expand(index);
        index = index.parent();
    }
    tree->setCurrentIndex(homeIndex);

    cxxdbg_connect(tree, &QTreeView::expanded, [tree](auto && ...) { tree->resizeColumnToContents(0); });
}


void FileSystemDockWidget::updateMenuStatus() {
}


}
