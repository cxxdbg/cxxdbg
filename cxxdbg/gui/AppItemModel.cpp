// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file AppItemModel.cpp
/// Contains implementations of Qt item models based on cxxdbg::app models.

#include "AppItemModel.h"
#include "cxxdbg/app/tree_view_model.hpp"
#include <QIcon>
#include <QDebug>
#include <iostream>
#include <QtWidgets/QMessageBox>
#include <QMessageBox>


namespace cxxdbg::gui {


AppRoItemModel::AppRoItemModel(const cxxdbg::ro_tree_view_model & mdl,
                               const ImageConverterSP & imgCvt, const ColorConverterSP & clrCvt):
model_{mdl},
imgCvt_{imgCvt},
clrCvt_{clrCvt} {

    before_added_con_ = mdl.before_added().connect([this](const auto & parent, auto first, auto last) {
        this->beginInsertRows(this->convertIndex(parent), first, last);
    });

    after_added_con_ = mdl.after_added().connect([this](auto && ...) {
        this->endInsertRows();
    });

    before_removed_con_ = mdl.before_removed().connect([this](const auto & parent, auto first, auto last) {
        this->beginRemoveRows(this->convertIndex(parent), first, last);
    });

    after_removed_con_ = mdl.after_removed().connect([this](auto && ...) {
        this->endRemoveRows();
    });

    after_changed_con_ = mdl.after_changed().connect([this](const auto & row) {
        emit this->dataChanged(this->convertIndex(row), this->convertIndex(row, 2));
    });

    after_changed_multy_con_ = mdl.after_changed_multy().connect([this](const auto& row_begin, const auto& row_end) {
        emit this->dataChanged(this->convertIndex(row_begin), this->convertIndex(row_end, 2));
    });

    before_layout_changed_con_ = mdl.before_layout_changed().connect([this](const auto & row) {
        this->onBeforeLayoutChanged(row);
    });

    after_layout_changed_con_ = mdl.after_layout_changed().connect([this](const auto & row) {
        this->onAfterLayoutChanged(row);
    });

    expand_row_con_ = mdl.expand_row().connect([this](const auto & row) {
        emit this->expandRow(this->convertIndex(row));
    });
}


AppRoItemModel::~AppRoItemModel() {
}


QModelIndex AppRoItemModel::index(int row, int column, const QModelIndex & parent) const {
    // sometimes Qt views can pass negative indexes
    if (row < 0) {
        return {};
    }

    auto parentRow = model_.make_index(parent.internalPointer());
    auto srow = static_cast<size_t>(row);

    // It looks like model should check for invalid row numbers and return
    // invalid indexes in such cases
    if (srow >= model_.childs_size(parentRow)) {
        return {};
    }

    return createIndex(row, column, model_.child(parentRow, srow).ptr());
}


QModelIndex AppRoItemModel::parent(const QModelIndex & child) const {
    auto row = model_.make_index(child.internalPointer());
    return convertIndex(model_.parent(row));
}


int AppRoItemModel::columnCount(const QModelIndex & parent) const {
    return static_cast<int>(model_.columns_size());
}


int AppRoItemModel::rowCount(const QModelIndex & parent) const {
    auto row = model_.make_index(parent.internalPointer());
    return static_cast<int>(model_.childs_size(row));
}


QVariant AppRoItemModel::data(const QModelIndex & index, int role) const {

    if (role == Qt::DecorationRole && imgCvt_ != nullptr) {
        auto mdl_idx = model_.make_index(index.internalPointer());
        size_t idx = model_.image(mdl_idx, index.column());
        if (idx != 0) {
            return imgCvt_->get(idx, mdl_idx);
        }
    }

    if (role == Qt::ForegroundRole && clrCvt_ != nullptr) {
        size_t idx = model_.color(model_.make_index(index.internalPointer()), index.column());
        if (idx != 0) {
            return QBrush{clrCvt_->get(idx)};
        }
    }

    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignLeft;
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return {};

    std::wstring s = model_.text(model_.make_index(index.internalPointer()), index.column());
    return QString::fromStdWString(s);
}


QVariant AppRoItemModel::headerData(int section, Qt::Orientation o, int role) const  {
    if (role != Qt::DisplayRole)
        return {};

    if (o != Qt::Horizontal)
        return {};

    return QString::fromStdWString(model_.column_name(section));
}


cxxdbg::tree_view_model_row_index AppRoItemModel::appRow(const QModelIndex & idx) const {
    return model_.make_index(idx.internalPointer());
}


void AppRoItemModel::onBeforeLayoutChanged(const AppRowIndex & row) {
    // saving old child indexes in the layoutChangedChilds_ map.
    // We will use this map for converting persistent indexes in the
    // after_layout_changed signal handler.

    assert(layoutChangedChilds_.empty() && "layoutChangedChilds_ is not empty");
    for (size_t i = 0, e = model_.childs_size(row); i < e; ++i) {
        layoutChangedChilds_.insert(std::make_pair(model_.child(row, i), i));
    }

    emit this->layoutAboutToBeChanged(QList<QPersistentModelIndex>{this->convertIndex(row)});
}


void AppRoItemModel::onAfterLayoutChanged(const AppRowIndex & row) {
    // updating persistent indexes of Qt model
    for (size_t i = 0, e = model_.childs_size(row); i < e; ++i) {
        auto newIdx = static_cast<int>(i);
        auto child = model_.child(row, i);

        auto oldIdxIt = layoutChangedChilds_.find(child);
        assert(oldIdxIt != layoutChangedChilds_.end() && "can't find child node");
        auto oldIdx = static_cast<int>(oldIdxIt->second);

        for (size_t j = 0; j < model_.columns_size(); ++j) {
            auto colIdx = static_cast<int>(j);
            auto oldMdlIndex = createIndex(oldIdx, colIdx, child.ptr());
            auto newMdlIndex = createIndex(newIdx, colIdx, child.ptr());

            changePersistentIndex(oldMdlIndex, newMdlIndex);
        }
    }


    // clearing the layoutChangedChilds_ map
    layoutChangedChilds_.clear();

    emit this->layoutChanged(QList<QPersistentModelIndex>{this->convertIndex(row)});
}

QModelIndex AppRoItemModel::convertIndex(const cxxdbg::tree_view_model_row_index & row,
                                         int column) const {
    if (!row)
        return {};

    return createIndex(static_cast<int>(model_.index(row)), column, row.ptr());
}

void AppRoItemModel::setImageConverter(AppRoItemModel::ImageConverterSP cvt) {
    imgCvt_ = cvt;
}

void AppRoItemModel::setColorConverter(AppRoItemModel::ColorConverterSP cvt) {
    clrCvt_ = cvt;
}

bool AppRoItemModel::hasExpandStateSupport() const {
    return model_.has_expand_state_support();
}

AppRoItemModel::ExpandState AppRoItemModel::getExpandState(const QModelIndex & index) const {
    return model_.get_expand_state(appRow(index));
}

void AppRoItemModel::setExpandState(const QModelIndex & index, ExpandStateSupport::ExpandState state) const {
    model_.set_expand_state(appRow(index), state);
}


void SortableAppRoItemModel::sort(int column, Qt::SortOrder order) {
    auto app_order = order == Qt::AscendingOrder ?
                     cxxdbg::sortable_tree_view_model::sort_order::ascending :
                     cxxdbg::sortable_tree_view_model::sort_order::descending ;
    model_.sort(static_cast<size_t>(column), app_order);
}


void SortableAppRoItemModel::disableSorting() {
    model_.sort(0, cxxdbg::sortable_tree_view_model::sort_order::no);
}



AppItemModel::AppItemModel(cxxdbg::tree_view_model & mdl):
AppRoItemModel{mdl},
model_{mdl} {
}


AppItemModel::~AppItemModel() {
}


Qt::ItemFlags AppItemModel::flags(const QModelIndex & index) const {
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (model_.editable(model_.make_index(index.internalPointer()), index.column()))
        flags |= Qt::ItemIsEditable;

    return flags;
}


bool AppItemModel::setData(const QModelIndex & index, const QVariant & value, int role) {
    if (role != Qt::EditRole)
        return false;

    std::wstring text = value.toString().toStdWString();
    model_.set_text(model_.make_index(index.internalPointer()), index.column(), text);
    return true;
}


}
