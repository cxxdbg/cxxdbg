// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file CallStackDockWidget.cpp
/// Contains implementation of CallStackDockWidget class.

#include "CallStackDockWidget.h"
#include "DebugUI.h"
#include "CxxdbgTreeView.h"
#include "cxxdbg_connect.hpp"
#include "cxxdbg/app/document_list.hpp"
#include "cxxdbg/dbg/call_stack_model.hpp"
#include "cxxdbg/dbg/debugger.hpp"
#include "cxxdbg/dbg/source_file.hpp"
#include "cxxdbg/dbg/stack_frame.hpp"
#include "cxxdbg/dbg/thread.hpp"

#include <filesystem>
#include <iomanip>
#include <sstream>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QSettings>
#include <QToolBar>
#include <QVBoxLayout>
#include <QFont>
#include <QFontMetrics>
#include <QItemDelegate>
#include <QIcon>

#include <QDebug>


namespace fs = std::filesystem;


namespace cxxdbg::gui {


namespace {
class CallStackImageConverter: public AppRoItemModel::ImageConverter {
public:
    CallStackImageConverter(DebugUI & dbgUi):
        dbgUi_{dbgUi} {}

    QIcon get(size_t idx, const cxxdbg::tree_view_model::row_index &) const override {
        switch (idx) {
        case cxxdbg::dbg::call_stack_model::call_stack_image_null_frame:
            return dbgUi_.rightYellowIcon();
        case cxxdbg::dbg::call_stack_model::call_stack_image_current_frame:
            return dbgUi_.rightBlueIcon();
        default:
            return {};
        }
    }

private:
    DebugUI & dbgUi_;
};


class CallStackItemDelegate: public QItemDelegate {
    //Q_OBJECT;
public:
    CallStackItemDelegate(AppRoItemModel & model, QObject * parent):
            QItemDelegate(parent), model_(model) {
    }

    void paint(QPainter * painter,
               const QStyleOptionViewItem & option,
               const QModelIndex & index) const override{
        if (index.column() != 0) {
            QItemDelegate::paint(painter, option, index);
            return;
        }

        drawBackground(painter, option, index);
        drawFocus(painter, option, option.rect);

        QVariant iconVariant = model_.data(index, Qt::DecorationRole);
        if (iconVariant.canConvert<QIcon>()) {
            QIcon icon = qvariant_cast<QIcon>(iconVariant);
            auto rect = option.rect;
            icon.paint(painter, rect, Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

private:
    AppRoItemModel & model_;
};
}


CallStackDockWidget::CallStackDockWidget(cxxdbg::dbg::debugger & dbg,
                                         cxxdbg::document_navigator & doc_nav,
                                         DebugUI & dbgUi,
                                         QWidget *parent):
CxxdbgDockWidget(tr("Call stack"), parent),
dbg_{dbg},
doc_nav_{doc_nav},
model_{dbg.call_stack()},
qtModel_{model_, std::make_shared<CallStackImageConverter>(dbgUi)} {
    setObjectName("call_stack");

    // switch to frame action
    QIcon switchToFrameIcon = dbgUi.makeSvgIcon("select");
    switchToFrameAction_ = new QAction{switchToFrameIcon, tr("Switch to frame"), this};
    switchToFrameAction_->setEnabled(false);
    cxxdbg_connect(switchToFrameAction_, &QAction::triggered, [this]() {
        // getting selected item
        QModelIndexList idxs = callStack_->selectionModel()->selectedRows();
        assert(idxs.size() == 1 && "single item should be selected");
        QModelIndex idx = idxs.at(0);

        // getting stack frame from mode index
        const cxxdbg::dbg::stack_frame * frame = model_.frame(qtModel_.appRow(idx));
        assert(frame != nullptr && "frame is null for selected frame item");

        // setting current stack frame
        dbg_.set_current_frame(frame);
    });

    // go to source action
    QIcon goToSourceIcon = dbgUi.makeSvgIcon("goto");
    goToSourceAction_ = new QAction{goToSourceIcon, tr("Go to source"), this};
    goToSourceAction_->setEnabled(false);
    cxxdbg_connect(goToSourceAction_, &QAction::triggered, [this]() {
        // getting selected item
        QModelIndexList idxs = callStack_->selectionModel()->selectedRows();
        assert(idxs.size() == 1 && "single item should be selected");
        QModelIndex idx = idxs.at(0);

        // getting stack frame from mode index
        const cxxdbg::dbg::stack_frame * frame = model_.frame(qtModel_.appRow(idx));
        assert(frame != nullptr && "frame is null for selected frame item");

        // show position in code viewer
        auto path = frame->pos().src_pos().file()->path();
        auto line = static_cast<size_t>(frame->pos().src_pos().line() - 1);
        doc_nav_.show_pos(path, line);
    });

    // creating icon for top frame
    int fontHeight = this->fontMetrics().height();
    QSize iconSize(fontHeight, fontHeight);
    topFrameIcon_ = QIcon(style()->standardPixmap(QStyle::SP_ArrowRight).scaled(iconSize));

    // creating tree view for list of stack frames
    {
        callStack_ = new CxxdbgTreeView("ui/call_stack/tree");
        setWidget(callStack_);
        callStack_->setFrameStyle(QFrame::NoFrame);
        callStack_->setModel(&qtModel_);
        auto * itemDeleagate = new CallStackItemDelegate(qtModel_, callStack_);
        callStack_->setItemDelegate(itemDeleagate);

        callStack_->setExpandsOnDoubleClick(false);
        
        int iconSectionSize = 16 * 2 + 5; // icon size + expand icon + space
        int numberSectionSize = QFontMetrics(callStack_->font()).horizontalAdvance(QString::number(999999));
        
        callStack_->setSectionSize(0, iconSectionSize);
        callStack_->setSectionSize(1, numberSectionSize);
        callStack_->restoreState();
    }
    
    bool isDispTypes = QSettings().value("ui/call_stack/display_types", true).toBool();
    bool isDispNames = QSettings().value("ui/call_stack/display_names", true).toBool();
    bool isDispVals = QSettings().value("ui/call_stack/display_values", true).toBool();

    // TODO move settings to application?
    model_.set_disp_par_types(isDispTypes);
    model_.set_disp_par_names(isDispNames);
    model_.set_disp_par_vals(isDispVals);


    // parameters display options actions

    displayTypesAction_ = new QAction(tr("Show parameter types"), this);
    displayTypesAction_->setCheckable(true);
    displayTypesAction_->setChecked(isDispTypes);
    cxxdbg_connect(displayTypesAction_, &QAction::triggered, [this] {
        model_.set_disp_par_types(displayTypesAction_->isChecked());
    });

    displayNamesAction_ = new QAction(tr("Show parameter names"), this);
    displayNamesAction_->setCheckable(true);
    displayNamesAction_->setChecked(isDispNames);
    cxxdbg_connect(displayNamesAction_, &QAction::triggered, [this] {
        model_.set_disp_par_names(displayNamesAction_->isChecked());
    });

    displayValuesAction_ = new QAction(tr("Show parameter values"), this);
    displayValuesAction_->setCheckable(true);
    displayValuesAction_->setChecked(isDispVals);
    cxxdbg_connect(displayValuesAction_, &QAction::triggered, [this] {
        model_.set_disp_par_vals(displayValuesAction_->isChecked());
    });


    // adding context menu for stack frame list

    {
        QAction * sep = new QAction(this);
        sep->setSeparator(true);
        callStack_->addAction(sep);
    }


    callStack_->addAction(switchToFrameAction_);
    callStack_->addAction(goToSourceAction_);

    {
        QAction * sep = new QAction(this);
        sep->setSeparator(true);
        callStack_->addAction(sep);
    }

    callStack_->addAction(displayTypesAction_);
    callStack_->addAction(displayNamesAction_);
    callStack_->addAction(displayValuesAction_);

    callStack_->setContextMenuPolicy(Qt::ActionsContextMenu);

    // enable actions depending on selection state
    cxxdbg_connect(callStack_->selectionModel(), &QItemSelectionModel::selectionChanged, [this]() {
        updateActions();
    });

    // after fetching all stack frames, state of actions may change
    cxxdbg_connect(&qtModel_, &QAbstractItemModel::dataChanged, [this](const auto & topLeft,
                                                                const auto & bottomRight,
                                                                const auto &) {
        QModelIndexList idxs = callStack_->selectionModel()->selectedRows();
        if (idxs.size() != 1)
            return;

        QModelIndex idx = idxs.at(0);
        if (topLeft.row() <= idx.row() && idx.row() <= bottomRight.row())
            this->updateActions();
    });

    // handle double click on list of frames
    cxxdbg_connect(callStack_, &QTreeView::doubleClicked, [this](const QModelIndex & idx) {
        if (switchToFrameAction_->isEnabled()) {
            switchToFrameAction_->trigger();
            return;
        }

        // fetching all stack frames if ... item is doubleclicked
        const cxxdbg::dbg::stack_frame * frame = model_.frame(qtModel_.appRow(idx));
        if (!frame) {
            dbg_.fetch_all_stack_frames();
        }
    });

    // disable call stack list if target is not in stopped state
    callStack_->setEnabled(false);
    dbg_.state_changed().connect([this] {
        callStack_->setEnabled(dbg_.state() == cxxdbg::dbg::debugger::state_t::stopped);
    });
}


void CallStackDockWidget::saveState() {
    callStack_->saveState();

    QSettings().setValue("ui/call_stack/display_types", QVariant(displayTypesAction_->isChecked()));
    QSettings().setValue("ui/call_stack/display_names", QVariant(displayNamesAction_->isChecked()));
    QSettings().setValue("ui/call_stack/display_values", QVariant(displayValuesAction_->isChecked()));
}


void CallStackDockWidget::updateActions() {
    QModelIndexList idxs = callStack_->selectionModel()->selectedRows();

    // enable go to source only if single frame is selected
    // and it's source position can be shown
    if (idxs.size() != 1) {
        goToSourceAction_->setEnabled(false);
        switchToFrameAction_->setEnabled(false);
    } else {
        QModelIndex idx = idxs.at(0);
        const cxxdbg::dbg::stack_frame * frame = model_.frame(qtModel_.appRow(idx));

        if (frame != nullptr) {
            switchToFrameAction_->setEnabled(true);

            if (frame->pos().src_pos().is_valid() &&
                fs::exists(frame->pos().src_pos().file()->path())) {
                goToSourceAction_->setEnabled(true);
            }
        } else {
            goToSourceAction_->setEnabled(false);
            switchToFrameAction_->setEnabled(false);
        }
    }
}


}
