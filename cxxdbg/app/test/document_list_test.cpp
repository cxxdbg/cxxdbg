// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file document_list_test.cpp
/// Contains unit tests for the document_list class.

#include "../memory_settings_storage.hpp"
#include "../document_list.hpp"
#include "cxxdbg/mock/mock.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::test {


/// Mock class for document_list_ui class
class mock_document_list_ui: public mock::object<document_list_ui>,
                             virtual public document_list_ui {
public:
    MOCK_DEFINE_METHOD_2_C(ask_open_big_file, bool (const std::filesystem::path &, size_t))
    MOCK_DEFINE_METHOD_2_C(ask_save_files, bool (const std::list<const file_document*> &, bool &))
};


/// Mock class for file document
class mock_file_document: public mock::object<mock_file_document>,
                          virtual public file_document {
public:
    mock_file_document(const path_t & p): file_document(p) {}

    MOCK_DEFINE_METHOD_0_C(can_undo, bool ())
    MOCK_DEFINE_METHOD_0_C(can_redo, bool ())
    MOCK_DEFINE_METHOD_0_C(can_cut, bool ())
    MOCK_DEFINE_METHOD_0_C(can_copy, bool ())
    MOCK_DEFINE_METHOD_0_C(can_paste, bool ())
    MOCK_DEFINE_METHOD_0_C(can_del, bool ())
    MOCK_DEFINE_METHOD_0_C(can_select_all, bool ())
    MOCK_DEFINE_METHOD_0_C(can_go_to_line, bool ())
    MOCK_DEFINE_METHOD_0(undo, void ())
    MOCK_DEFINE_METHOD_0(redo, void ())
    MOCK_DEFINE_METHOD_0(cut, std::wstring ())
    MOCK_DEFINE_METHOD_0(copy, std::wstring ())
    MOCK_DEFINE_METHOD_1(paste, void (const std::wstring &))
    MOCK_DEFINE_METHOD_0(del, void ())
    MOCK_DEFINE_METHOD_0(select_all, void ())
    MOCK_DEFINE_METHOD_1(go_to_line, void (size_t))
    MOCK_DEFINE_METHOD_0_C(lines_count, size_t ())

    MOCK_DEFINE_METHOD_0_C(changed, bool ())
    MOCK_DEFINE_METHOD_0(save, void ())
    MOCK_DEFINE_METHOD_1(save_as, void (const path_t &))
};


struct document_list_test_fixture {
    mock_document_list_ui ui;
    memory_settings_storage sett;
    document_list docs{ui, sett, "xfiles", false};
};


BOOST_FIXTURE_TEST_SUITE(document_list_test, document_list_test_fixture)


/// Tests asking and saving file before close it
BOOST_AUTO_TEST_CASE(test_save_before_close) {
    mock_file_document * doc = nullptr;
    docs.open_file("/my/path", true, [&doc] {
        auto d = std::make_unique<mock_file_document>("/my/path");
        doc = d.get();
        return std::move(d);
    });

    MOCK_ADD_CALL(*doc, changed, [] {
        return true;
    });

    int n_save_called = 0;
    MOCK_ADD_CALL(*doc, save, [&n_save_called] {
        ++n_save_called;
    });

    MOCK_ADD_CALL(ui, ask_save_files, [doc](auto && files, bool & save) {
        BOOST_CHECK_EQUAL(files.size(), 1);
        BOOST_CHECK(files.front() == doc);
        save = true;
        return true;
    });

    BOOST_CHECK(docs.close(doc));

    // can't call doc->verity to verify mock calls because doc is destroyed at this point
    BOOST_CHECK_EQUAL(n_save_called, 1);

    BOOST_CHECK(ui.verify());
}


/// Tests asking and dropping file changes
BOOST_AUTO_TEST_CASE(test_close_no_save) {
    mock_file_document * doc = nullptr;
    docs.open_file("/my/path", true, [&doc] {
        auto d = std::make_unique<mock_file_document>("/my/path");
        doc = d.get();
        return std::move(d);
    });

    MOCK_ADD_CALL(*doc, changed, [] {
        return true;
    });

    MOCK_ADD_CALL(ui, ask_save_files, [doc](auto && files, bool & save) {
        BOOST_CHECK_EQUAL(files.size(), 1);
        BOOST_CHECK(files.front() == doc);
        save = false;
        return true;
    });

    BOOST_CHECK(docs.close(doc));

    BOOST_CHECK(ui.verify());
}



/// Tests cancelling file close
BOOST_AUTO_TEST_CASE(test_close_cancel) {
    mock_file_document * doc = nullptr;
    docs.open_file("/my/path", true, [&doc] {
        auto d = std::make_unique<mock_file_document>("/my/path");
        doc = d.get();
        return std::move(d);
    });

    MOCK_ADD_CALL(*doc, changed, [] {
        return true;
    });

    MOCK_ADD_CALL(ui, ask_save_files, [doc](auto && files, bool & save) {
        BOOST_CHECK_EQUAL(files.size(), 1);
        BOOST_CHECK(files.front() == doc);
        return false;
    });

    BOOST_CHECK(!docs.close(doc));

    BOOST_CHECK(ui.verify());
}



BOOST_AUTO_TEST_SUITE_END()


}
