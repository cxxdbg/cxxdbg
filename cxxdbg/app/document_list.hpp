// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file document_list.hpp
/// Contains definition of the document_list class.

#pragma once

#include "document.hpp"
#include "file_document.hpp"
#include "recent_file_list.hpp"
#include "text_model_file_document.hpp"
#include "cxxdbg/log/log.hpp"
#include "cxxdbg/util/signals.hpp"
#include <ranges.hpp>
#include <filesystem>
#include <list>


namespace cxxdbg {


class file_document;
class settings_storage;
class text_document;


/// Delegate for UI requests from document list
class document_list_ui {
public:
    /// Virtual destructor
    virtual ~document_list_ui() = default;

    /// Asks user for opening big file with specified path and size
    virtual bool ask_open_big_file(const std::filesystem::path & p, size_t sz) const = 0;

    /// Asks user for saving modified file. Returns false if user cancelled action.
    virtual bool ask_save_files(const std::list<const file_document*> & files, bool & save) const = 0;
};


/// List of opened documents in application.
/// For now can contain only text_file_document objects in list.
class document_list {
public:
    /// Type of document path
    using path_t = std::filesystem::path;

    /// Constructs empty document list
    document_list(const document_list_ui & ui,
                  settings_storage & sett,
                  const std::string & recent_files_name,
                  bool is_r_o);

    /// Destroys document list
    ~document_list() = default;

    // not copyable/moveable/assignable
    document_list(const document_list &) = delete;
    document_list(document_list &&) = delete;
    document_list & operator=(const document_list &) = delete;
    document_list & operator=(document_list &&) = delete;


    ////////////////////////////////////////////////////////////
    // Document management

    /// Returns const range of all opened documents
    auto all() const {
        auto fn = [](auto && ptr) { return static_cast<const document*>(ptr.get()); };
        return docs_ | std::ranges::views::transform(fn);
    }

    /// Returns range of all opened documents
    auto all() {
        auto fn = [](auto && ptr) { return ptr.get(); };
        return docs_ | std::ranges::views::transform(fn);
    }

    /// Returns range of all file documents
    auto files() {
        auto filter_fn = [](auto && doc) {
            return dynamic_cast<file_document*>(doc) != nullptr;
        };
        auto cast_fn = [](auto && doc) {
            return dynamic_cast<file_document*>(doc);
        };

        return all() | std::ranges::views::filter(filter_fn) | std::ranges::views::transform(cast_fn);
    }

    /// Returns range of all text documents
    auto texts() {
        auto filter_fn = [](auto && doc) {
            return dynamic_cast<text_document*>(doc) != nullptr;
        };
        auto cast_fn = [](auto && doc) {
            return dynamic_cast<text_document*>(doc);
        };

        return all() | std::ranges::views::filter(filter_fn) | std::ranges::views::transform(cast_fn);
    }

    /// Opens file document with specified path using specified create function if document is not opened yet.
    template <typename CreateFn>
    auto open_file(const path_t & p, bool add_recent, const CreateFn & create_fn) {
        using doc_type = std::decay_t<decltype(*create_fn())>;

        CXXDBG_LOG_INFO(app) << "open document: " << p;

        // looking for existing opened document
        auto & doc = file_docs_[p];
        if (!doc) {
            // creating new text file document
            {
                auto new_doc = create_fn();
                doc = new_doc.get();

                // adding document list of opened documents
                docs_.push_back(std::move(new_doc));

                // connecting to document signals and mapping them to list signals
                // Note: we don't need scoped connection because document lifetime is always
                // less than document list lifetime

                doc->name_changed().connect([this, doc] {
                    document_name_changed(doc);
                });

                doc->changed_changed().connect([this, doc] {
                    document_changed_changed(doc);
                    if (doc == current()) {
                        can_save_changed();
                    }
                });
            }

            // notifying clients about new opened file
            document_opened(doc);
        }

        // setting opened source file as current
        set_current(doc);

        // adding file to list of recent files
        if (add_recent) {
            recent_files_.add(p);
        }

        auto res = dynamic_cast<doc_type*>(doc);
        assert(res && "existing file document must be of requested type");
        return res;
    }

    /// Opens specified text file if not opened yet and makes it current.
    /// Returns text document for opened file
    template <bool ReadOnly>
    text_model_file_document<ReadOnly> * open_text(const path_t & p, bool add_recent);

    /// Checks text file size, ask user confirmation if size is too large, and
    /// then opens text file and adds it into list of recent files if add_recent is true
    template <bool ReadOnly>
    void check_size_and_open_text_file(const std::filesystem::path & p, bool add_recent);

    /// Closes specified document. Returns false if action was cancelled by user
    bool close(document * doc);

    /// Closes current document. Returns false if action was cancelled by user
    bool close();

    /// Closes all opened documents. Returns false if action was cancelled by user
    bool close_all();

    /// Sets current opened document
    void set_current(document * doc);

    /// Returns current opened document or nullptr if there are no opened documents
    document * current() { return curr_doc_; }

    /// Returns const pointer to current opened document or nullptr if no documents opened
    const document * current() const { return curr_doc_; }

    /// Returns current opened text document or nullptr if there are not opened
    /// documents or opened document is not a text document
    text_document * current_text();

    /// Returns const pointer to current opened text document or nullptr if
    /// there are not opened documents or opened document is not a text document
    const text_document * current_text() const;

    /// Opens text file document with specified path if not opened or set
    /// opened document as current, and sets position in document to line with specified
    /// number
    template <bool ReadOnly>
    void show_pos(const std::filesystem::path & p, size_t line);

    /// The signal is emitted after new document is opened
    signal<void (document*)> document_opened;

    /// The signal is emitted before document closed
    signal<void (document*)> document_closed;

    /// The signal is emitted after position in document is changed
    signal<void (text_document*)> document_pos_changed;

    /// The signal is emitted after "changed" flag of document is changed
    signal<void (file_document*)> document_changed_changed;

    /// The signal is emitted after document name is changed
    //CXXDBG_DEFINE_SIGNALX(document_name_changed, void(document*))
    signal<void (document*)> document_name_changed;

    /// The signal is emitted after current document is changed
    signal<void ()> current_changed;


    ////////////////////////////////////////////////////////////
    // Document editing

    /// Returns true if can perform undo in current document
    bool can_undo() const { return current() && current()->can_undo(); }

    /// Returns true if can perform redo in current document
    bool can_redo() const { return current() && current()->can_redo(); }

    /// Returns true if can perform cut in current document
    bool can_cut() const { return current() && current()->can_cut(); }

    /// Returns true if can perform copy in current document
    bool can_copy() const { return current() && current()->can_copy(); }

    /// Returns true if can perform paste in current document
    bool can_paste() const { return current() && current()->can_paste(); }

    /// Returns true if can perform delete in current document
    bool can_del() const { return current() && current()->can_del(); }

    /// Returns true if can perform select all in current document
    bool can_select_all() const { return current() && current()->can_select_all(); }

    /// Returns true if can perform go to line in current document
    bool can_go_to_line() const { return current() && current()->can_go_to_line(); }


    /// Performs undo in current document
    void undo() {
        assert(can_undo() && "can't do undo");
        current()->undo();
    }

    /// Performs redo in current document
    void redo() {
        assert(can_redo() && "can't do redo");
        current()->redo();
    }

    /// Performs cut in current  document.
    /// Returns string that should be inserted into clipboard
    std::wstring cut() {
        assert(can_cut() && "can't do cut");
        return current()->cut();
    }

    /// Performs copy in current document.
    /// Returns string that should be inserted into clipboard
    std::wstring copy() {
        assert(can_copy() && "can't do copy");
        return current()->copy();
    }

    /// Performs paste in document
    void paste(const std::wstring & text) {
        assert(can_paste() && "can't do paste");
        current()->paste(text);
    }

    /// Performs delete in current document.
    void del() {
        assert(can_del() && "can't do del");
        current()->del();
    }

    /// Performs select all in current document
    void select_all() {
        assert(can_select_all() && "can't do select all");
        current()->select_all();
    }

    /// Performs go to line in current document
    void go_to_line(size_t l) {
        assert(can_go_to_line() && "can't go to line");
        current()->go_to_line(l);
    }

    /// Returns number of lines in current document for go to line function
    size_t lines_count() const {
        assert(current() && "can't call lines_count without current document");
        return current()->lines_count();
    }

    /// The signal is emitted when current document edition options changed
    signal<void()> current_edit_options_changed;


    ////////////////////////////////////////////////////////////
    // Document change flags and saving

    /// Returns true if current document can be saved and was changes after last save
    bool can_save() const;

    /// Saves current document
    void save();

    /// Returns true if current document can be saved
    bool can_save_as() const;

    /// Saves current document as
    void save_as(const path_t & p);

    /// Returns default save path for current document
    path_t default_save_path() const;

    /// Returns range of all changed file documents
    auto changed_files() {
        auto fn = [](auto && doc) { return doc->changed(); };
        return files() | std::ranges::views::filter(fn);
    }

    /// Saves all changed documents
    void save_all();

    /// Checks if all files saved. If there are some unsaved files then asks user
    /// about saving files and save them if user answers yes. Returns false
    /// if user cancelled saving files
    bool ask_save_files();

    /// The signal is emitted when can_save return value changes
    signal<void()> can_save_changed;

    /// The signal is emitted when can_save_as return value changes
    signal<void()> can_save_as_changed;

    /// Returns reference to list of recent files
    auto & recent_files() { return recent_files_; }

    /// Returns current symbol under cursor
    std::wstring symbol_under_cursor() const;

    /// The signal is emitted when current symbol under cursor changes
    CXXDBG_DEFINE_SIGNALX(symbol_under_cursor_changed, void())

private:
    /// Reference to UI delegate
    const document_list_ui & ui_;

    /// List of opened documents
    std::list<std::unique_ptr<document>> docs_;

    /// Map of opened file documents
    std::map<path_t, file_document*> file_docs_;

    document * curr_doc_;               ///< Current opened document
    bool all_docs_read_only_ = false;   ///< Open all documents for read only
    recent_file_list recent_files_;     ///< List of recent files

    scoped_signal_connection edit_options_changed_con_;
};


template <bool ReadOnly>
text_model_file_document<ReadOnly> * document_list::open_text(const path_t & p, bool add_recent) {
    return open_file(p, add_recent, [this, p] {
        // creating new text file document
        auto new_doc = std::make_unique<text_model_file_document<ReadOnly>>(p);

        // connecting to position change signal of text document
        auto doc_ptr = new_doc.get();
        new_doc->text_view_model().selection().changed.connect([this, doc_ptr] {
            document_pos_changed(doc_ptr);
            symbol_under_cursor_changed()();
        });

        return std::move(new_doc);
    });
}


template <bool ReadOnly>
void document_list::check_size_and_open_text_file(const std::filesystem::path & p, bool add_recent) {
    // getting file size
    auto sz = std::filesystem::file_size(p);
    if (sz == static_cast<boost::uintmax_t>(-1)) {
        std::ostringstream msg;
        msg << "Can't get size of file '" << p.string() << "'";
        throw std::runtime_error{msg.str()};
    }

    // checking file size and ask confirmation is file size is too large (> 1 Mb)
    if (sz > 1024 * 1024 * 1) {
        if (!ui_.ask_open_big_file(p, sz)) {
            return;
        }
    }

    // opening file
    open_text<ReadOnly>(p, add_recent);
}


template <bool ReadOnly>
void document_list::show_pos(const std::filesystem::path & p, size_t line) {
    // opening file if not opened
    auto doc = open_text<ReadOnly>(p, false);

    // setting current file
    set_current(doc);

    // setting file position or move cursor to the end if no such
    // source position in opened file
    auto lnum = line;
    if (doc->lines_count() < line) {
        lnum = doc->lines_count() - 1;
    }

    doc->go_to_line(lnum);
}


}


