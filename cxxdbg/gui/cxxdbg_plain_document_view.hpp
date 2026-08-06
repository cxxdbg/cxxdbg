// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file cxxdbg_plain_document_view_base.hpp
/// Contains definition of the cxxdbg_plain_document_view_base class.

#pragma once

#include "tmvc/basic_text_view_model.hpp"
#include "tmvc/preserving_viewport_position_model.hpp"
#include "tmvc/qt/native_plain_text_view.hpp"
#include "tmvc/qt/impl/menu.hpp"
#include <QContextMenuEvent>
#include <QMenu>
#include <QTextBlock>


namespace cxxdbg::gui {


/// Base class for plain view widgets used for displaying text documents in debugger and IDE.
/// TODO: move all line numbers and marks panel logic into text library and remove this class.
class cxxdbg_plain_document_view_base {
public:
    /// Type of delegate function for adding custom context menu items
    using custom_context_menu_delegate = std::function<void (QMenu*)>;

    /// Default virtual destructor
    virtual ~cxxdbg_plain_document_view_base() = default;

    /// Returns first visible block in plain text edit
    virtual QTextBlock firstVisibleBlock() const = 0;

    /// Returns block bounding rect in content coordinates
    virtual QRectF blockBoundingGeometry(const QTextBlock & block) const = 0;

    /// Returns block bounding rect in block coordinates
    virtual QRectF blockBoundingRect(const QTextBlock &block) const = 0;

    /// Returns content offset
    virtual QPointF contentOffset() const = 0;

    /// Sets delegate for adding custom context menu
    virtual void set_custom_context_menu_delegate(const custom_context_menu_delegate & delegate) = 0;
};


/// Plain text view with additional functions
template <
    tmvc::text_model TextModel,
    tmvc::selection_model SelectionModel,
    tmvc::selection_controller_for<TextModel> Controller
>
class cxxdbg_plain_document_view:
    public tmvc::qt::native_plain_text_view<TextModel, SelectionModel, Controller>,
    public cxxdbg_plain_document_view_base
{
    /// Type of text view model
    using text_view_model_t = tmvc::basic_text_view_model <
        TextModel,
        SelectionModel,
        tmvc::preserving_viewport_position_model<TextModel, SelectionModel>,
        Controller
    >;

    /// Type of base class
    using base_type = tmvc::qt::native_plain_text_view<TextModel, SelectionModel, Controller>;

public:
    /// Constructs editor view with specified model and parent
    cxxdbg_plain_document_view(text_view_model_t & mdl, QWidget * parent = nullptr):
        base_type{mdl.text(), mdl.selection(), mdl.controller()},
        controller_{mdl.controller()} {
        this->setParent(parent);
    }

    /// Returns first visible block in plain text edit
    QTextBlock firstVisibleBlock() const override {
        return base_type::firstVisibleBlock();
    }

    /// Returns block bounding rect in content coordinates
    QRectF blockBoundingGeometry(const QTextBlock & block) const override {
        return base_type::blockBoundingGeometry(block);
    }

    /// Returns block bounding rect in block coordinates
    QRectF blockBoundingRect(const QTextBlock &block) const override {
        return base_type::blockBoundingRect(block);
    }

    /// Returns content offset
    QPointF contentOffset() const override {
        return base_type::contentOffset();
    }

    /// Sets delegate for adding custom context menu
    void set_custom_context_menu_delegate(const custom_context_menu_delegate & delegate) override {
        context_menu_delegate_ = delegate;
    }

protected:
    /// Displays context menu built from the controller, with custom actions added
    /// via the context menu delegate
    void contextMenuEvent(QContextMenuEvent * event) override {
        auto menu = tmvc::qt::impl::create_qt_menu_from_context_menu<typename TextModel::char_t>(
            controller_.create_context_menu(), controller_);

        if (context_menu_delegate_) {
            context_menu_delegate_(menu.get());
        }

        menu->exec(event->globalPos());
    }

private:
    Controller & controller_;                               ///< Reference to controller
    custom_context_menu_delegate context_menu_delegate_;     ///< Custom context menu delegate
};


}
