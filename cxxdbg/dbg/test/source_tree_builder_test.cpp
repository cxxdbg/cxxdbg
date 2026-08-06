// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

//
// Created by yzarudny on 29.05.17.
//

/// \file source_tree_builder_test.cpp
/// Contains unit tests for watch_list class.

#include "cxxdbg/dbg/source_model.hpp"
#include "cxxdbg/dbg/source_tree_builder.hpp"
#include <boost/test/unit_test.hpp>
#include <filesystem>


namespace cxxdbg::dbg::test {


BOOST_AUTO_TEST_SUITE(source_tree_builder_test_suite)
namespace {
struct Fixture {
    // Fixture seem to be created anew each case...
    // so use singletons
    source_model & sources() {
        static source_model sources_;
        return sources_;
    };

    source_tree_builder & builder() {
        static source_tree_builder builder_;
        return builder_;
    };
};
}


#ifdef _WIN32
static const std::filesystem::path root_path = "C:\\";
#else
static const std::filesystem::path root_path = "/";
#endif


/// Tests adding single root node to watch list
BOOST_FIXTURE_TEST_CASE(add_single_file, Fixture) {

    // add file
    builder().add( sources().source(std::filesystem::path(root_path / "opt/work/test1.cpp")) );

    BOOST_CHECK(builder().tree().subdirs_count() == 1);

    source_tree_builder_directory & missing_dir = builder().tree();

    auto root_dir_it = missing_dir.find_subdir(root_path.wstring());
    BOOST_REQUIRE(root_dir_it != missing_dir.subdirs_end());

    auto opt_dir_it = root_dir_it->find_subdir(L"opt");
    BOOST_REQUIRE(opt_dir_it != root_dir_it->subdirs_end());

    auto work_dir_it = opt_dir_it->find_subdir(L"work");
    BOOST_REQUIRE(work_dir_it != opt_dir_it->subdirs_end());

    BOOST_CHECK(work_dir_it->name() == L"work");
    BOOST_CHECK(work_dir_it->files_count() == 1);

    auto test1_cpp_it = work_dir_it->find_file(L"test1.cpp");
    BOOST_REQUIRE(test1_cpp_it != work_dir_it->files_end());

    const source_file * test1_cpp_ptr = * test1_cpp_it;
    BOOST_REQUIRE(test1_cpp_ptr != nullptr);
    BOOST_CHECK(test1_cpp_ptr->name() == L"test1.cpp");
}

// ensure that adding the same existing file does nothing
BOOST_FIXTURE_TEST_CASE(add_the_same_file, Fixture) {
    BOOST_CHECK(builder().tree().subdirs_count() == 1);

    // add the same existing file
    builder().add( sources().source(std::filesystem::path(root_path / "opt/work/test1.cpp")) );

    BOOST_CHECK(builder().tree().subdirs_count() == 1);

    source_tree_builder_directory & missing_dir = builder().tree();

    auto root_dir_it = missing_dir.find_subdir(root_path.wstring());
    BOOST_REQUIRE(root_dir_it != missing_dir.subdirs_end());
    BOOST_CHECK(root_dir_it->name() == root_path.wstring());

    auto opt_dir_it = root_dir_it->find_subdir(L"opt");
    BOOST_REQUIRE(opt_dir_it != root_dir_it->subdirs_end());
    BOOST_CHECK(opt_dir_it->name() == L"opt");

    auto work_dir_it = opt_dir_it->find_subdir(L"work");
    BOOST_REQUIRE(work_dir_it != opt_dir_it->subdirs_end());
    BOOST_CHECK(work_dir_it->name() == L"work");

    BOOST_CHECK(work_dir_it->files_count() == 1);

    auto test1_cpp_it = work_dir_it->find_file(L"test1.cpp");
    BOOST_REQUIRE(test1_cpp_it != work_dir_it->files_end());

    const source_file * test1_cpp_ptr = * test1_cpp_it;
    BOOST_REQUIRE(test1_cpp_ptr != nullptr);
    BOOST_CHECK(test1_cpp_ptr->name() == L"test1.cpp");
}


/// Tests adding directory into source tree (should be ignored)
BOOST_FIXTURE_TEST_CASE(add_dir, Fixture) {
    source_tree_builder b;
#ifdef _WIN32
    b.add(sources().source(std::filesystem::path("C:/Users")));
#else
    b.add(sources().source(std::filesystem::path("/usr")));
#endif
    BOOST_CHECK_EQUAL(b.tree().subdirs_count(), 0);
}


BOOST_AUTO_TEST_SUITE_END()


}
