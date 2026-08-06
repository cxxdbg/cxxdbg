// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file text_model_document.hpp
/// Contains definition of the text_model_document class.

#pragma once

#include "text_document.hpp"
#include "tmvc/basic_text_view_model.hpp"
#include "tmvc/preserving_viewport_position_model.hpp"
#include "tmvc/signals.hpp"


namespace cxxdbg {


/// Represents text document which displays text from text model
template <
    tmvc::text_model TextModel,
    tmvc::selection_model SelectionModel,
    tmvc::selection_controller_for<TextModel> Controller
>
class text_model_document: virtual public text_document {
    /// Is document read only
    static constexpr auto is_editable = tmvc::edit_controller<Controller>;

public:
    /// Type of text view model for displaying text
    using text_view_model_t = tmvc::basic_text_view_model <
        TextModel,
        SelectionModel,
        tmvc::preserving_viewport_position_model<TextModel, SelectionModel>,
        Controller
    >;

    /// Type of text model
    using text_model_t = TextModel;

    /// Type of selection model
    using selection_model_t = SelectionModel;

    /// Type of controller
    using controller_t = Controller;


    /// Constructs document with specified parameters passed to text model constructor
    template <typename ... Args>
    requires (std::constructible_from<TextModel, Args...>)
    explicit text_model_document(Args && ... args):
    doc_{std::forward<Args>(args)...} {
        // notifying user about can_* options change after text selection
        // and undo/redo change
        pos_changed_con_ = doc_.selection().changed.connect([this] {
            this->edit_options_changed()();
        });

        if constexpr (is_editable) {
            can_undo_changed_con_ = doc_.controller().history().can_undo_changed.connect([this] {
                this->edit_options_changed()();
            });
            can_redo_changed_con_ = doc_.controller().history().can_redo_changed.connect([this] {
                this->edit_options_changed()();
            });
        }
    }


    /// Returns reference to document model
    auto & text_view_model() {
        return doc_;
    }


    /// Returns const reference to document model
    const auto & text_view_model() const {
        return doc_;
    }


    /// Returns true if can perform undo in document
    bool can_undo() const override {
        if constexpr (is_editable) {
            return doc_.controller().can_undo();
        } else {
            return false;
        }
    }


    /// Returns true if can perform redo in document
    bool can_redo() const override {
        if constexpr (is_editable) {
            return doc_.controller().can_redo();
        } else {
            return false;
        }
    }


    /// Returns true if can perform cut action in document
    bool can_cut() const override {
        if constexpr (is_editable) {
            return doc_.controller().can_cut();
        } else {
            return false;
        }
    }


    /// Returns true if can perform paste action in document
    bool can_paste() const override {
        return is_editable;
    }


    /// Returns true if can perform delete action in document
    bool can_del() const override {
        if constexpr (is_editable) {
            return doc_.controller().can_delete();
        } else {
            return false;
        }
    }


    /// Performs undo action in document
    void undo() override {
        if constexpr (is_editable) {
            doc_.controller().undo();
        } else {
            assert(false && "can't perform undo in read only document");
        }
    }


    /// Performs redo action in document
    void redo() override {
        if constexpr (is_editable) {
            doc_.controller().redo();
        } else {
            assert(false && "can't perform redo in read only document");
        }
    }


    /// Performs cut action in document. Returns string that should be inserted into clipboard
    std::wstring cut() override {
        if constexpr (is_editable) {
            auto chars = doc_.controller().cut();
            return std::wstring{chars.begin(), chars.end()};
        } else {
            assert(false && "can't perform cut in read only document");
            return {};
        }
    }


    /// Performs paste action in document
    void paste(const std::wstring & text) override {
        if constexpr (is_editable) {
            doc_.controller().paste(text);
        } else {
            assert(false && "can't perform paste action in read only document");
        }
    }


    /// Performs delete action in document.
    void del() override {
        if constexpr (is_editable) {
            doc_.controller().delete_();
        } else {
            assert(is_editable && "can't perform delete action in read only document");
        }
    }


    /// Sets tab size for document
    void set_tab_size(unsigned val) override {
        if constexpr (is_editable) {
            doc_.controller().set_tab_size(val);
        }
    }


    /// Sets whether tabs should be expanded to spaces in document
    void set_expand_tabs(bool val) override {
        if constexpr (is_editable) {
            doc_.controller().set_expand_tabs(val);
        }
    }


    /// Returns true if can perform copy action in document
    bool can_copy() const override {
        return doc_.controller().can_copy();
    }


    /// Performs copy action in document. Returns string that should be inserted into clipboard
    std::wstring copy() override {
        auto chars = doc_.controller().copy();
        return std::wstring{chars.begin(), chars.end()};
    }


    /// Performs select all action in document
    void select_all() override {
        doc_.controller().select_all();
    }


    /// Performs go to line in document
    void go_to_line(size_t l) override {
        // updating line number only if not same
        if (doc_.selection().pos().line == l) {
            return;
        }

        doc_.controller().select_text({l, 0}, {l, 0});
    }


    /// Returns number of lines in document for go to line function
    size_t lines_count() const override {
        return doc_.text().lines_size();
    }


    /// Returns string at specified line
    std::wstring line_str(size_t l) const override {
        return tmvc::line_str(doc_.text(), l);
    }


    /// Returns current position in text document
    tmvc::position current_pos() const override {
        return doc_.selection().pos();
    }


private:
    text_view_model_t doc_;         ///< Text view model

    // connection to document model signals
    tmvc::scoped_signal_connection pos_changed_con_;
    tmvc::scoped_signal_connection can_undo_changed_con_;
    tmvc::scoped_signal_connection can_redo_changed_con_;
};


}
