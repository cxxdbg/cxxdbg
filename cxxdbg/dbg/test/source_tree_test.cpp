// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file source_tree_test.cpp
/// Contains unit tests for the source_tree class.

#include "../source_model.hpp"
#include "../source_tree.hpp"
#include "../source_tree_builder.hpp"
#include <boost/test/unit_test.hpp>


namespace fs = std::filesystem;


namespace cxxdbg::dbg::test {


struct source_tree_test_fixture {
    source_model sources_;
    source_tree tree;
};


#ifdef _WIN32
static const fs::path root_path = "C:\\";
#else
static const fs::path root_path = "/";
#endif


BOOST_FIXTURE_TEST_SUITE(source_tree_test, source_tree_test_fixture)


/// Tests source tree construction
BOOST_AUTO_TEST_CASE(ctor) {
    BOOST_CHECK_EQUAL(tree.childs_size(nullptr), 0);
}


/// Tests merging to empty tree
BOOST_AUTO_TEST_CASE(merge_to_empty) {
    source_tree_builder_directory root;
    auto it = root.add_subdir(root_path.wstring());
    it = it->add_subdir(L"dir");
    it->add_file(fs::path{L"bar"}, sources_.source(root_path / L"dir" / L"bar"));
    it->add_file(fs::path{L"foo"}, sources_.source(root_path / L"dir" / L"foo"));
    tree.merge(root);

    BOOST_REQUIRE_EQUAL(tree.childs_size(nullptr), 1);
    auto r = tree.child(nullptr, 0);
    BOOST_REQUIRE(r);

    BOOST_REQUIRE_EQUAL(tree.childs_size(r), 1);
    auto dir = tree.child(r, 0);
    BOOST_REQUIRE(dir);

    BOOST_REQUIRE_EQUAL(tree.childs_size(dir), 2);

    auto bar = tree.child(dir, 0);
    BOOST_REQUIRE(bar);
    BOOST_CHECK(bar->name() == L"bar");

    auto foo = tree.child(dir, 1);
    BOOST_REQUIRE(foo);
    BOOST_CHECK(foo->name() == L"foo");
}


/// Tests merging content into existing directory
BOOST_AUTO_TEST_CASE(merge_existing_dir) {
    {
        source_tree_builder_directory root;
        auto it = root.add_subdir(root_path.wstring());
        it = it->add_subdir(L"dir");
        it->add_file(fs::path{L"bar"}, sources_.source(root_path / L"dir/bar"));
        it->add_file(fs::path{L"foo"}, sources_.source(root_path / L"dir/foo"));
        tree.merge(root);
    }

    {
        source_tree_builder_directory root;
        auto it = root.add_subdir(root_path.wstring());
        
        it = it->add_subdir(L"dir");
        it->add_file(fs::path{L"bar"}, sources_.source(root_path / L"dir/bar"));
        it->add_file(fs::path{L"zzz"}, sources_.source(root_path / L"dir/zzz"));

        it = it->add_subdir(L"sdir2");
        it->add_file(fs::path{"aaa"}, sources_.source(root_path / L"dir/sdir2/aaa"));

        tree.merge(root);
    }


    BOOST_REQUIRE_EQUAL(tree.childs_size(nullptr), 1);
    auto r = tree.child(nullptr, 0);
    BOOST_REQUIRE(r);

    BOOST_REQUIRE_EQUAL(tree.childs_size(r), 1);
    auto dir = tree.child(r, 0);
    BOOST_REQUIRE(dir);

    BOOST_REQUIRE_EQUAL(tree.childs_size(dir), 4);

    auto bar = tree.child(dir, 1);
    BOOST_REQUIRE(bar);
    BOOST_CHECK(bar->name() == L"bar");

    auto foo = tree.child(dir, 2);
    BOOST_REQUIRE(foo);
    BOOST_CHECK(foo->name() == L"foo");

    auto zzz = tree.child(dir, 3);
    BOOST_REQUIRE(zzz);
    BOOST_CHECK(zzz->name() == L"zzz");

    auto sdir2 = tree.child(dir, 0);
    BOOST_REQUIRE(sdir2);
    BOOST_CHECK(sdir2->name() == L"sdir2");
    BOOST_REQUIRE_EQUAL(tree.childs_size(sdir2), 1);

    auto aaa = tree.child(sdir2, 0);
    BOOST_REQUIRE(aaa);
    BOOST_CHECK(aaa->name() == L"aaa");
    BOOST_CHECK_EQUAL(tree.childs_size(aaa), 0);
}


/// Tests adding subdirectory with same name and suffix
BOOST_AUTO_TEST_CASE(add_subdir_with_suffix) {
    {
        source_tree_builder_directory root;
        auto it = root.add_subdir(root_path.wstring());
        it = it->add_subdir(L"dir");
        it->add_file(fs::path{L"bar"}, sources_.source(root_path / L"dir/bar"));
        it->add_file(fs::path{L"foo"}, sources_.source(root_path / L"dir/foo"));
        tree.merge(root);
    }

    {
        source_tree_builder_directory root;
        auto it = root.add_subdir(root_path.wstring());
        
        it = it->add_subdir(L"dir2");
        it->add_file(fs::path{L"zzz"}, sources_.source(root_path / L"dir2/zzz"));

        tree.merge(root);
    }


    BOOST_REQUIRE_EQUAL(tree.childs_size(nullptr), 1);
    auto r = tree.child(nullptr, 0);
    BOOST_REQUIRE(r);

    BOOST_REQUIRE_EQUAL(tree.childs_size(r), 2);
    auto dir = tree.child(r, 0);
    BOOST_REQUIRE(dir);

    BOOST_REQUIRE_EQUAL(tree.childs_size(dir), 2);

    auto bar = tree.child(dir, 0);
    BOOST_REQUIRE(bar);
    BOOST_CHECK(bar->name() == L"bar");

    auto foo = tree.child(dir, 1);
    BOOST_REQUIRE(foo);
    BOOST_CHECK(foo->name() == L"foo");

    auto dir2 = tree.child(r, 1);
    BOOST_REQUIRE(dir2);

    BOOST_REQUIRE_EQUAL(tree.childs_size(dir2), 1);
    auto zzz = tree.child(dir2, 0);
    BOOST_REQUIRE(zzz);
    BOOST_CHECK(zzz->name() == L"zzz");
}


/// Tests searching for item for path
BOOST_AUTO_TEST_CASE(test_path_item) {
    {
        source_tree_builder_directory root;
        auto it = root.add_subdir(root_path.wstring());
        it = it->add_subdir(L"dir");
        it->add_file(fs::path{L"bar"}, sources_.source(root_path / L"dir" / L"bar"));
        tree.merge(root);
    }

    auto item = tree.path_item(root_path / L"dir" / L"bar");
    BOOST_CHECK(item != nullptr);
}


BOOST_AUTO_TEST_SUITE_END()


}
