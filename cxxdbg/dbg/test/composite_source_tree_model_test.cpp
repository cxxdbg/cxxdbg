// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file composite_source_tree_model_test.cpp
/// Contains unit tests for composite_source_tree_model class.


#include "../composite_source_tree_model.hpp"
#include "../source_tree.hpp"
#include <boost/test/unit_test.hpp>
#include <memory>


namespace fs = std::filesystem;


namespace cxxdbg::dbg::test {


struct composite_source_tree_model_test_fixture {
    source_tree sources;
    composite_source_tree_model mdl{sources};
};


#ifdef _WIN32
static const std::wstring root_path_str = L"C:\\";
#else
static const std::wstring root_path_str = L"/";
#endif

const fs::path root_path = root_path_str;


BOOST_FIXTURE_TEST_SUITE(composite_source_tree_model_test, composite_source_tree_model_test_fixture)


/// Tests construction
BOOST_AUTO_TEST_CASE(ctor) {
    BOOST_CHECK_EQUAL(mdl.childs_size({}), 0);
}


/// Tests adding directory with no merged chains
BOOST_AUTO_TEST_CASE(add_dir_simple) {
    int n_before_added_called = 0;
    mdl.before_added().connect([this, &n_before_added_called](auto && parent, auto first, auto last) {
        BOOST_CHECK_EQUAL(n_before_added_called, 0);
        ++n_before_added_called;

        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 0);
        BOOST_CHECK_EQUAL(mdl.childs_size({}), 0);
    });

    int n_after_added_called = 0;
    mdl.after_added().connect([this, &n_after_added_called](auto && parent, auto first, auto last) {
        BOOST_CHECK_EQUAL(n_after_added_called, 0);
        ++n_after_added_called;

        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 0);

        BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 1);

        auto root = mdl.child({}, 0);
        BOOST_CHECK(mdl.text(root, 0) == root_path_str);

        BOOST_REQUIRE_EQUAL(mdl.childs_size(root), 1);
        auto file = mdl.child(root, 0);
        BOOST_CHECK(mdl.text(file, 0) == L"file");
    });

    mdl.before_removed().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    mdl.after_removed().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    mdl.after_changed().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    auto dir = std::make_unique<source_tree_item>(root_path, true);
    dir->insert(0, std::make_unique<source_tree_item>(root_path / L"file", false));
    sources.insert_node(nullptr, 0, std::move(dir));

    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 1);
    auto root = mdl.child({}, 0);
    BOOST_CHECK(mdl.text(root, 0) == root_path_str);

    BOOST_REQUIRE_EQUAL(mdl.childs_size(root), 1);
    auto file = mdl.child(root, 0);
    BOOST_CHECK(mdl.text(file, 0) == L"file");

    BOOST_CHECK_EQUAL(n_before_added_called, 1);
    BOOST_CHECK_EQUAL(n_after_added_called, 1);
}


/// Tests adding directory with merged chains
BOOST_AUTO_TEST_CASE(add_dir_merged) {
    int n_before_added_called = 0;
    mdl.before_added().connect([this, &n_before_added_called](auto && parent, auto first, auto last) {
        BOOST_CHECK_EQUAL(n_before_added_called, 0);
        ++n_before_added_called;

        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 0);

        BOOST_CHECK_EQUAL(mdl.childs_size({}), 0);
    });

    int n_after_added_called = 0;
    mdl.after_added().connect([this, &n_after_added_called](auto && parent, auto first, auto last) {
        BOOST_CHECK_EQUAL(n_after_added_called, 0);
        ++n_after_added_called;

        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 0);

        BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 1);

        auto root = mdl.child({}, 0);
        BOOST_CHECK(mdl.text(root, 0) == (root_path / L"dir").wstring());

        BOOST_REQUIRE_EQUAL(mdl.childs_size(root), 1);
        auto file = mdl.child(root, 0);
        BOOST_CHECK(mdl.text(file, 0) == L"file");
    });

    mdl.before_removed().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    mdl.after_removed().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    mdl.after_changed().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    {
        auto root = std::make_unique<source_tree_item>(root_path, true);
        auto dir = std::make_unique<source_tree_item>(root_path / "dir", true);
        dir->insert(0, std::make_unique<source_tree_item>(root_path / "dir" / "file", false));
        root->insert(0, std::move(dir));
        sources.insert_node(nullptr, 0, std::move(root));
    }

    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 1);
    auto root = mdl.child({}, 0);
    BOOST_CHECK(mdl.text(root, 0) == (root_path / L"dir").wstring());

    BOOST_REQUIRE_EQUAL(mdl.childs_size(root), 1);
    auto file = mdl.child(root, 0);
    BOOST_CHECK(mdl.text(file, 0) == L"file");

    BOOST_CHECK_EQUAL(n_before_added_called, 1);
    BOOST_CHECK_EQUAL(n_after_added_called, 1);
}


/// Tests extending chain after adding directory at the end of chain
BOOST_AUTO_TEST_CASE(extend_chain) {
    {
        auto root = std::make_unique<source_tree_item>(root_path, true);
        auto dir = std::make_unique<source_tree_item>(root_path / L"dir", true);
        auto subdir = std::make_unique<source_tree_item>(root_path / L"dir" / L"subdir", true);
        dir->insert(0, std::move(subdir));
        root->insert(0, std::move(dir));

        auto dir2 = std::make_unique<source_tree_item>(root_path / L"dir2", true);
        root->insert(1, std::move(dir2));

        sources.insert_node(nullptr, 0, std::move(root));
    }

    int n_before_removed_called = 0;
    mdl.before_removed().connect([this, &n_before_removed_called](auto && parent, auto first, auto last) {
        BOOST_CHECK_EQUAL(n_before_removed_called, 0);
        ++n_before_removed_called;

        // model should emulate removing dir/subdir node chain

        BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 1);
        auto root = mdl.child({}, 0);

        BOOST_CHECK(parent == root);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 0);

        BOOST_REQUIRE_EQUAL(mdl.childs_size(root), 2);

        auto subdir = mdl.child(root, 0);
        BOOST_CHECK(mdl.text(subdir, 0) == (fs::path{L"dir"} / L"subdir").wstring());
        BOOST_CHECK_EQUAL(mdl.childs_size(subdir), 0);

        auto dir2 = mdl.child(root, 1);
        BOOST_CHECK(mdl.text(dir2, 0) == L"dir2");
        BOOST_CHECK_EQUAL(mdl.childs_size(dir2), 0);
    });

    int n_after_removed_called = 0;
    mdl.after_removed().connect([this, &n_after_removed_called](auto && parent, auto first, auto last) {
        BOOST_CHECK_EQUAL(n_after_removed_called, 0);
        ++n_after_removed_called;

        // model should emulate removing dir/subdir node chain

        BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 1);
        auto root = mdl.child({}, 0);

        BOOST_CHECK(parent == root);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 0);

        BOOST_CHECK_EQUAL(mdl.childs_size(root), 1);
        auto dir2 = mdl.child(root, 0);
        BOOST_CHECK(mdl.text(dir2, 0) == L"dir2");
        BOOST_CHECK_EQUAL(mdl.childs_size(dir2), 0);
    });


    int n_before_added_called = 0;
    mdl.before_added().connect([this, &n_before_added_called](auto && parent, auto first, auto last) {
        BOOST_CHECK_EQUAL(n_before_added_called, 0);
        ++n_before_added_called;

        // model should emulate removing dir/subdir node chain.
        // Emulation should be in effect in before handler

        BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 1);
        auto root = mdl.child({}, 0);

        BOOST_CHECK(parent == root);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 0);

        BOOST_CHECK_EQUAL(mdl.childs_size(root), 1);
        auto dir2 = mdl.child(root, 0);
        BOOST_CHECK(mdl.text(dir2, 0) == L"dir2");
        BOOST_CHECK_EQUAL(mdl.childs_size(dir2), 0);
    });

    int n_after_added_called = 0;
    mdl.after_added().connect([this, &n_after_added_called](auto && parent, auto first, auto last) {
        BOOST_CHECK_EQUAL(n_after_added_called, 0);
        ++n_after_added_called;

        BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 1);
        auto root = mdl.child({}, 0);

        BOOST_CHECK(parent == root);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 0);

        BOOST_CHECK(mdl.text(root, 0) == root_path_str);

        BOOST_REQUIRE_EQUAL(mdl.childs_size(root), 2);

        auto subdir2 = mdl.child(root, 0);
        BOOST_CHECK(mdl.text(subdir2, 0) == (fs::path{L"dir"} / L"subdir" / L"subdir2").wstring());

        BOOST_REQUIRE_EQUAL(mdl.childs_size(subdir2), 1);
        auto file = mdl.child(subdir2, 0);
        BOOST_CHECK(mdl.text(file, 0) == L"file");

        auto dir2 = mdl.child(root, 1);
        BOOST_CHECK(mdl.text(dir2, 0) == L"dir2");
        BOOST_CHECK_EQUAL(mdl.childs_size(dir2), 0);
    });

    mdl.after_changed().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    {
        BOOST_REQUIRE_EQUAL(sources.childs_size(nullptr), 1);
        auto root = sources.child(nullptr, 0);
        BOOST_REQUIRE(root);
    
        BOOST_REQUIRE_EQUAL(sources.childs_size(root), 2);
        auto dir = sources.child(root, 0);

        BOOST_REQUIRE_EQUAL(sources.childs_size(dir), 1);
        auto subdir = sources.child(dir, 0);

        auto subdir2 = std::make_unique<source_tree_item>(root_path / L"dir" / L"subdir" / L"subdir2", true);
        subdir2->insert(0, std::make_unique<source_tree_item>(root_path / L"dir" / L"subdir" / L"subdir2" / L"file", false));
        sources.insert_node(subdir, 0, std::move(subdir2));
    }

    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 1);
    auto root = mdl.child({}, 0);
    BOOST_CHECK(mdl.text(root, 0) == root_path_str);

    BOOST_REQUIRE_EQUAL(mdl.childs_size(root), 2);

    auto subdir2 = mdl.child(root, 0);
    BOOST_CHECK(mdl.text(subdir2, 0) == (fs::path{L"dir"} / L"subdir" / L"subdir2").wstring());

    BOOST_REQUIRE_EQUAL(mdl.childs_size(subdir2), 1);
    auto file = mdl.child(subdir2, 0);
    BOOST_CHECK(mdl.text(file, 0) == L"file");

    auto dir2 = mdl.child(root, 1);
    BOOST_CHECK(mdl.text(dir2, 0) == L"dir2");
    BOOST_CHECK_EQUAL(mdl.childs_size(dir2), 0);

    BOOST_CHECK_EQUAL(n_before_removed_called, 1);
    BOOST_CHECK_EQUAL(n_after_removed_called, 1);
    BOOST_CHECK_EQUAL(n_before_added_called, 1);
    BOOST_CHECK_EQUAL(n_after_added_called, 1);
}


/// Tests splitting chain after adding directory in the middle of chain
BOOST_AUTO_TEST_CASE(split_chain) {
    {
        auto root = std::make_unique<source_tree_item>(root_path, true);
        auto dir = std::make_unique<source_tree_item>(root_path / L"dir", true);
        auto subdir = std::make_unique<source_tree_item>(root_path / L"dir" / L"subdir", true);
        subdir->insert(0, std::make_unique<source_tree_item>(root_path / L"dir" / L"subdir" / L"file", false));
        dir->insert(0, std::move(subdir));
        root->insert(0, std::move(dir));

        auto dir2 = std::make_unique<source_tree_item>(root_path / L"dir2", true);
        root->insert(1, std::move(dir2));

        sources.insert_node(nullptr, 0, std::move(root));
    }

    int n_before_removed_called = 0;
    mdl.before_removed().connect([this, &n_before_removed_called](auto && parent, auto first, auto last) {
        BOOST_CHECK_EQUAL(n_before_removed_called, 0);
        ++n_before_removed_called;

        // model should emulate removing dir/subdir node chain

        BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 1);
        auto root = mdl.child({}, 0);

        BOOST_CHECK(parent == root);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 0);

        BOOST_REQUIRE_EQUAL(mdl.childs_size(root), 2);

        auto subdir = mdl.child(root, 0);
        BOOST_CHECK(mdl.text(subdir, 0) == (fs::path{L"dir"} / L"subdir").wstring());

        BOOST_REQUIRE_EQUAL(mdl.childs_size(subdir), 1);
        auto file = mdl.child(subdir, 0);
        BOOST_CHECK(mdl.text(file, 0) == L"file");
        BOOST_CHECK_EQUAL(mdl.childs_size(file), 0);

        auto dir2 = mdl.child(root, 1);
        BOOST_CHECK(mdl.text(dir2, 0) == L"dir2");
        BOOST_CHECK_EQUAL(mdl.childs_size(dir2), 0);
    });

    int n_after_removed_called = 0;
    mdl.after_removed().connect([this, &n_after_removed_called](auto && parent, auto first, auto last) {
        BOOST_CHECK_EQUAL(n_after_removed_called, 0);
        ++n_after_removed_called;

        // model should emulate removing dir/subdir node chain

        BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 1);
        auto root = mdl.child({}, 0);

        BOOST_CHECK(parent == root);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 0);

        BOOST_REQUIRE_EQUAL(mdl.childs_size(root), 1);

        auto dir2 = mdl.child(root, 0);
        BOOST_CHECK(mdl.text(dir2, 0) == L"dir2");
        BOOST_CHECK_EQUAL(mdl.childs_size(dir2), 0);
    });


    int n_before_added_called = 0;
    mdl.before_added().connect([this, &n_before_added_called](auto && parent, auto first, auto last) {
        BOOST_CHECK_EQUAL(n_before_added_called, 0);
        ++n_before_added_called;

        // model should emulate removing dir/subdir node chain.
        // Emulation should be in effect in before handler

        BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 1);
        auto root = mdl.child({}, 0);

        BOOST_CHECK(parent == root);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 0);

        BOOST_REQUIRE_EQUAL(mdl.childs_size(root), 1);

        auto dir2 = mdl.child(root, 0);
        BOOST_CHECK(mdl.text(dir2, 0) == L"dir2");
        BOOST_CHECK_EQUAL(mdl.childs_size(dir2), 0);
    });

    int n_after_added_called = 0;
    mdl.after_added().connect([this, &n_after_added_called](auto && parent, auto first, auto last) {
        BOOST_CHECK_EQUAL(n_after_added_called, 0);
        ++n_after_added_called;

        BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 1);
        auto root = mdl.child({}, 0);

        BOOST_CHECK(parent == root);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 0);

        BOOST_REQUIRE_EQUAL(mdl.childs_size(root), 2);

        auto dir = mdl.child(root, 0);
        BOOST_CHECK(mdl.text(dir, 0) == L"dir");

        BOOST_REQUIRE_EQUAL(mdl.childs_size(dir), 2);

        auto subdir = mdl.child(dir, 0);
        BOOST_CHECK(mdl.text(subdir, 0) == L"subdir");

        BOOST_REQUIRE_EQUAL(mdl.childs_size(subdir), 1);
        auto file = mdl.child(subdir, 0);
        BOOST_CHECK(mdl.text(file, 0) == L"file");
        BOOST_CHECK_EQUAL(mdl.childs_size(file), 0);

        auto subdir2 = mdl.child(dir, 1);
        BOOST_CHECK(mdl.text(subdir2, 0) == L"subdir2");

        BOOST_REQUIRE_EQUAL(mdl.childs_size(subdir2), 1);
        auto file2 = mdl.child(subdir2, 0);
        BOOST_CHECK(mdl.text(file2, 0) == L"file2");
        BOOST_CHECK_EQUAL(mdl.childs_size(file2), 0);

        auto dir2 = mdl.child(root, 1);
        BOOST_CHECK(mdl.text(dir2, 0) == L"dir2");
        BOOST_CHECK_EQUAL(mdl.childs_size(dir2), 0);
    });

    mdl.after_changed().connect([](auto && ... args) {
        BOOST_CHECK(false);
    });

    {
        BOOST_REQUIRE_EQUAL(sources.childs_size(nullptr), 1);
        auto root = sources.child(nullptr, 0);
        BOOST_REQUIRE(root);
    
        BOOST_REQUIRE_EQUAL(sources.childs_size(root), 2);
        auto dir = sources.child(root, 0);

        auto subdir2 = std::make_unique<source_tree_item>(root_path / L"dir" / L"subdir2", true);
        subdir2->insert(0, std::make_unique<source_tree_item>(root_path / L"dir" / L"subdir2" / L"file2", false));
        sources.insert_node(dir, 1, std::move(subdir2));
    }

    BOOST_REQUIRE_EQUAL(mdl.childs_size({}), 1);
    auto root = mdl.child({}, 0);
    BOOST_CHECK(mdl.text(root, 0) == root_path_str);

    BOOST_REQUIRE_EQUAL(mdl.childs_size(root), 2);

    auto dir = mdl.child(root, 0);
    BOOST_CHECK(mdl.text(dir, 0) == L"dir");

    BOOST_REQUIRE_EQUAL(mdl.childs_size(dir), 2);

    auto subdir = mdl.child(dir, 0);
    BOOST_CHECK(mdl.text(subdir, 0) == L"subdir");

    BOOST_REQUIRE_EQUAL(mdl.childs_size(subdir), 1);
    auto file = mdl.child(subdir, 0);
    BOOST_CHECK(mdl.text(file, 0) == L"file");
    BOOST_CHECK_EQUAL(mdl.childs_size(file), 0);

    auto subdir2 = mdl.child(dir, 1);
    BOOST_CHECK(mdl.text(subdir2, 0) == L"subdir2");

    BOOST_REQUIRE_EQUAL(mdl.childs_size(subdir2), 1);
    auto file2 = mdl.child(subdir2, 0);
    BOOST_CHECK(mdl.text(file2, 0) == L"file2");
    BOOST_CHECK_EQUAL(mdl.childs_size(file2), 0);

    auto dir2 = mdl.child(root, 1);
    BOOST_CHECK(mdl.text(dir2, 0) == L"dir2");
    BOOST_CHECK_EQUAL(mdl.childs_size(dir2), 0);

    BOOST_CHECK_EQUAL(n_before_removed_called, 1);
    BOOST_CHECK_EQUAL(n_after_removed_called, 1);
    BOOST_CHECK_EQUAL(n_before_added_called, 1);
    BOOST_CHECK_EQUAL(n_after_added_called, 1);
}



BOOST_AUTO_TEST_SUITE_END()


}
