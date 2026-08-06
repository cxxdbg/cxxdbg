// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file document_list.cpp
/// Contains implementation of the document_list.cpp

#include "document_list.hpp"
#include "text_file_document.hpp"
#include <sstream>


namespace cxxdbg {


document_list::document_list(const document_list_ui & ui,
                             settings_storage & sett,
                             const std::string & recent_files_name,
                             bool is_r_o):
ui_{ui},
curr_doc_{nullptr},
all_docs_read_only_{is_r_o},
recent_files_{sett, recent_files_name} {
}


bool document_list::close(document * doc) {
    // checking if file was modified
    auto fdoc = dynamic_cast<file_document*>(doc);
    if (fdoc) {
        if (fdoc->changed()) {
            bool do_save = false;
            if (!ui_.ask_save_files({fdoc}, do_save)) {
                // user cancelled action
                return false;
            }

            if (do_save) {
                fdoc->save();
            }
        }
    }

    // if doc is current source then changing current source to
    // the first source from the list of opened sources
    if (curr_doc_ == doc) {
        if (docs_.size() == 1) {
            curr_doc_ = nullptr;

            // disconnecting from edit options change signal
            edit_options_changed_con_.disconnect();

            current_edit_options_changed();
            can_save_changed();
            can_save_as_changed();
            current_changed();
            symbol_under_cursor_changed();
        } else {
            set_current(docs_.front().get());
        }
    }

    document_closed(doc);

    // removing file document from list of opened file documents
    if (auto tf_doc = dynamic_cast<file_document*>(doc)) {
        file_docs_.erase(tf_doc->path());
    }

    // removing source from the list of opened sources
    auto it = std::find_if(docs_.begin(), docs_.end(), [doc](auto && ptr) {
        return ptr.get() == doc;
    });
    assert(it != docs_.end() && "Document is not opened");
    docs_.erase(it);

    return true;
}


bool document_list::close() {
    assert(current() && "no current opened document");
    return close(current());
}


bool document_list::close_all() {
    while (!docs_.empty()) {
        close(docs_.front().get());
    }

    return true;
}


void document_list::set_current(document * doc) {
    assert(doc != nullptr && "can't set current document to null");

    if (curr_doc_ == doc)
        return;

    curr_doc_ = doc;
    current_changed();

    edit_options_changed_con_ = curr_doc_->edit_options_changed().connect([this] {
        current_edit_options_changed();
    });

    current_edit_options_changed();
    can_save_changed();
    can_save_as_changed();
    symbol_under_cursor_changed();
}


text_document * document_list::current_text() {
    return dynamic_cast<text_document*>(current());
}


const text_document *document_list::current_text() const {
    return dynamic_cast<const text_document*>(current());
}


bool document_list::can_save() const {
    if (!can_save_as()) {
        return false;
    }

    auto fdoc = dynamic_cast<const file_document*>(current());
    assert(fdoc && "not a file document, but can_save_as returned true");
    return fdoc->changed();
}


void document_list::save() {
    auto fdoc = dynamic_cast<file_document*>(current());
    assert(fdoc && "current document is not a file document");
    fdoc->save();
}


bool document_list::can_save_as() const {
    return dynamic_cast<const file_document*>(current()) != nullptr;
}


void document_list::save_as(const document_list::path_t & p) {
    auto fdoc = dynamic_cast<file_document*>(current());
    assert(fdoc && "current document is not a file document");
    fdoc->save_as(p);
}


document_list::path_t document_list::default_save_path() const {
    auto fdoc = dynamic_cast<const file_document*>(current());
    assert(fdoc && "current document is not a file document");
    return fdoc->path();
}


void document_list::save_all() {
    for (auto doc : changed_files()) {
        doc->save();
    }
}


bool document_list::ask_save_files() {
    if (!changed_files().empty()) {
        auto files = changed_files();
        auto files_list = std::list<const file_document*>{std::ranges::begin(files), std::ranges::end(files)};
        bool save = false;
        if (!ui_.ask_save_files(files_list, save)) {
            // user cancelled action
            return false;
        }

        if (save) {
            // user answered to save all files
            save_all();
        }
    }

    return true;
}


namespace {
inline bool is_valid_symbol_char(wchar_t c) {
    return std::isalnum(c) || c == '_' || c == '$';
}
}


std::wstring document_list::symbol_under_cursor() const {
    // getting current opened document and position in it
    auto doc = current_text();
    if (!doc) {
        return {};
    }

    auto pos = doc->current_pos();
    const auto & line = doc->line_str(pos.line);

    int l = pos.column;
    --l;
    int r = pos.column;

    while (l >= 0 && is_valid_symbol_char(line[l]))
        --l;

    while (r < line.length() && is_valid_symbol_char(line[r]))
        ++r;

    std::wstring symbol;

    int len = r - l - 1;
    if (len > 0) {
        symbol = line.substr(static_cast<unsigned int>(l + 1), static_cast<unsigned int>(len));
    }

    return symbol;
}


}
