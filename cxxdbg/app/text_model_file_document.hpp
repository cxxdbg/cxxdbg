// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file text_model_file_document.hpp
/// Contains definition of the text_model_file_document class.

#pragma once

#include "tmvc/simple_text_model.hpp"
#include "tmvc/simple_single_selection_model.hpp"
#include "tmvc/single_edit_controller.hpp"
#include "text_file_document.hpp"
#include "text_model_document.hpp"
#include <codecvt>
#include <sstream>


namespace cxxdbg {


/// Base class for text_file_document
template <bool ReadOnly>
using text_model_file_document_base = std::conditional_t<
    ReadOnly,
    text_model_document <
        tmvc::wsimple_text_model,
        tmvc::simple_single_selection_model<tmvc::wsimple_text_model>,
        tmvc::single_selection_controller<tmvc::wsimple_text_model>
    >,
    text_model_document <
        tmvc::wsimple_text_model,
        tmvc::simple_single_selection_model<tmvc::wsimple_text_model>,
        tmvc::single_edit_controller<tmvc::wsimple_text_model>
    >
>;


/// Text file document implemented using text model
template <bool ReadOnly = false>
class text_model_file_document: virtual public text_file_document,
                                public text_model_file_document_base<ReadOnly> {
    /// Type of base class
    using base_type = text_model_file_document_base<ReadOnly>;

public:
    /// Constructs text file document with specified file path
    /// and document content
    text_model_file_document(const path_t & p, const std::wstring & t):
    file_document{p} {
        init();
    }


    /// Constructs text file document with specified file path and reads its content
    text_model_file_document(const path_t & p):
    file_document{p} {
        std::wifstream ifstr{p.string(), std::ios::in | std::ios::binary};

        if (!ifstr.is_open()) {
            std::ostringstream msg;
            msg << "Can't open file " << p << " for reading";
            throw std::runtime_error{msg.str()};
        }

        // use UTF-8 character converted for system C locale
        std::locale loc("");
        if (loc.name() == "C") {
            loc = std::locale{loc, new std::codecvt_utf8<wchar_t>};
        }

        ifstr.imbue(loc);
        std::wstring wstr;
        while(true) {
            wchar_t ch;
            ifstr.read(&ch, 1);
            if (!ifstr) {
                break;
            }

            wstr.push_back(ch);
        }

        this->text_view_model().text().reset(wstr);

        init();
    }


    /// Returns true if document was changed in application after last save
    bool changed() const override {
        if constexpr (ReadOnly) {
            return false;
        } else {
            return this->text_view_model().controller().history().changed();
        }
    }


    /// Saves document
    void save() override {
        save_as(path());
    }


    /// Saves document as another file
    void save_as(const path_t & p) override {
        if constexpr (!ReadOnly) {
            // performing modifications in text model that is required before saving doc
            this->text_view_model().controller().do_before_save();
            
            std::wofstream ofstr{p.string(), std::ios::out | std::ios::binary};

            if (!ofstr.is_open()) {
                std::ostringstream msg;
                msg << "Can't open file " << p << " for writing";
                throw std::runtime_error{msg.str()};
            }

            ofstr.imbue(std::locale(""));
            std::wstring wstr = tmvc::string(this->text_view_model().text());
            ofstr.write(&wstr[0], wstr.size());

            if (!ofstr) {
                std::ostringstream msg;
                msg << "Error while writing file " << p;
                throw std::runtime_error{msg.str()};
            }

            ofstr.close();

            this->text_view_model().controller().history().clear_changed();
            set_path(p);
        } else {
            assert(false && "save_as can't be called for read only document");
        }
    }


private:
    /// Performs additional document initialization
    void init() {
        // listening for change of the document change flag
        if constexpr (!ReadOnly) {
            changed_changed_con_ = this->text_view_model().controller().history().changed_changed.connect([this] {
                this->changed_changed()();
            });
        }
    }


    tmvc::scoped_signal_connection changed_changed_con_;
};


}
