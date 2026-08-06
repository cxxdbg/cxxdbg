// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file AppItemModel.h
/// Contains definitions of Qt item models based on cxxdbg::app models.

#pragma once

#include "cxxdbg/app/signals.hpp"
#include "cxxdbg/app/tree_view_model.hpp"

#include <QAbstractItemModel>
#include <QColor>

#include <unordered_map>


namespace cxxdbg::gui {


class ExpandStateSupport {
public:
    using ExpandState = cxxdbg::tree_view_expand_state;
    /// Returns true if model has expand_state interface support and false otherwise
    virtual bool hasExpandStateSupport() const = 0;

    /// Returns expand state for selected index
    virtual ExpandState getExpandState(const QModelIndex & index) const = 0;

    /// Sets expand state for seleceted index
    virtual void setExpandState(const QModelIndex & index, ExpandState state) const = 0;
};


/// Qt item model for read only cxxdbg::app model
class AppRoItemModel: public QAbstractItemModel, public ExpandStateSupport {
    Q_OBJECT
    using AppRowIndex = cxxdbg::tree_view_model::row_index;
public:
    /// Abstract image converter for model
    class ImageConverter {
    public:
        /// Destructor, destroys object
        virtual ~ImageConverter() {}

        /// Converts image index to icon
        virtual QIcon get(size_t idx, const cxxdbg::tree_view_model::row_index & row) const = 0;
    };

    class ColorConverter {
    public:
        /// Destructor, destroys object
        virtual ~ColorConverter() {}

        /// Converts color index to color
        virtual QColor get(size_t idx) const = 0;
    };

    typedef std::shared_ptr<ImageConverter> ImageConverterSP;
    typedef std::shared_ptr<ColorConverter> ColorConverterSP;


    /// Constructor, makes model with specified reference to cxxdbg::app model
    /// and optional image converted
    AppRoItemModel(const cxxdbg::ro_tree_view_model & mdl,
                   const ImageConverterSP & imgCvt = {}, const ColorConverterSP & clrCvt = {});

    /// Destructor, destroys object
    virtual ~AppRoItemModel();

    /// Returns index of item
    QModelIndex index(int row, int column, const QModelIndex & parent) const override;

    /// Returns index of item parent
    QModelIndex parent(const QModelIndex & child) const override;

    /// Retrns number of column count
    int columnCount(const QModelIndex & parent) const override;

    /// Returns number of child rows
    int rowCount(const QModelIndex & parent) const override;

    /// Returns data for item with specified index
    QVariant data(const QModelIndex & index, int role) const override;

    /// Returns header data
    QVariant headerData(int section, Qt::Orientation o, int role) const override;

    /// Returns app row index for model index
    cxxdbg::tree_view_model_row_index appRow(const QModelIndex & idx) const;

    /// Sets image converter
    void setImageConverter(ImageConverterSP cvt);

    /// Sets color converter
    void setColorConverter(ColorConverterSP cvt);

    ///// ExpandStateSupport interface implementation //////

    /// Returns true if model has expand_state interface support and false otherwise
    bool hasExpandStateSupport() const override;

    /// Returns expand state for selected index
    ExpandState getExpandState(const QModelIndex & index) const override;

    /// Sets expand state
    void setExpandState(const QModelIndex & index, ExpandState state) const override;

    /// Converts tree_view_model_row_index to Qt model index
    QModelIndex convertIndex(const AppRowIndex & row, int column = 0) const;

signals:
    void expandRow(const QModelIndex & index);

private:
    /// Called before layout of child items changed
    void onBeforeLayoutChanged(const AppRowIndex & row);

    /// Called after layout of child items changed
    void onAfterLayoutChanged(const AppRowIndex & row);

    const cxxdbg::ro_tree_view_model & model_;        ///< Reference to application model
    ImageConverterSP imgCvt_;                       ///< Pointer to image converter
    ColorConverterSP clrCvt_;                       ///< Pointer to color converter

    /// Map of indexes of child nodes for updating in the layout changed
    /// signal handler
    std::unordered_map<AppRowIndex, size_t> layoutChangedChilds_;

    // signal connections
    cxxdbg::scoped_signal_connection before_added_con_;
    cxxdbg::scoped_signal_connection after_added_con_;
    cxxdbg::scoped_signal_connection before_removed_con_;
    cxxdbg::scoped_signal_connection after_removed_con_;
    cxxdbg::scoped_signal_connection after_changed_con_;
    cxxdbg::scoped_signal_connection after_changed_multy_con_;
    cxxdbg::scoped_signal_connection before_layout_changed_con_;
    cxxdbg::scoped_signal_connection after_layout_changed_con_;
    cxxdbg::scoped_signal_connection expand_row_con_;
};


// Sortable Qt item model for read only app model
class SortableAppRoItemModel: virtual public AppRoItemModel {
public:
    /// Constructor sortable Qt model from app read only model
    SortableAppRoItemModel(cxxdbg::sortable_ro_tree_view_model & mdl):
        AppRoItemModel{mdl}, model_{mdl} {}

    virtual ~SortableAppRoItemModel() = default;

    /// Sorts model
    void sort(int column, Qt::SortOrder) override;

    /// Disables sorting
    void disableSorting();

private:
    cxxdbg::sortable_ro_tree_view_model & model_;     ///< Reference to model
};


/// Qt item model for editable cxxdbg::app model
class AppItemModel: virtual public AppRoItemModel {
public:
    /// Constructor, makes model with specified reference to cxxdbg::app model
    AppItemModel(cxxdbg::tree_view_model & mdl);

    /// Destructor, destroys object
    virtual ~AppItemModel();

    /// Returns flags for item with specified index
    Qt::ItemFlags flags(const QModelIndex & index) const override;

    /// Sets data for item with specified index
    bool setData(const QModelIndex & index, const QVariant & value, int role) override;

private:
    cxxdbg::tree_view_model & model_;         ///< Reference to application model
};


/// Sortable Qt item model for editable cxxdbg::app model
class SortableAppItemModel: public SortableAppRoItemModel,
                            public AppItemModel {
public:
    /// Constructors sortable Qt model with specified reference to cxxdbg::app model
    SortableAppItemModel(cxxdbg::sortable_tree_view_model & mdl):
        AppRoItemModel{mdl}, SortableAppRoItemModel{mdl}, AppItemModel{mdl} {}

    virtual ~SortableAppItemModel() = default;
};


}
