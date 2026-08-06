// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file recent_file_list_tedt.cpp
/// Contains unit tests for the recent_file_list class.

#include "cxxdbg/app/recent_file_list.hpp"
#include "cxxdbg/app/memory_settings_storage.hpp"
#include <boost/test/unit_test.hpp>
#include <ranges.hpp>


namespace cxxdbg::dbg::test {


struct recent_file_list_test_fixture {
    memory_settings_storage sett;
    recent_file_list lst{sett, "mylist", 5};
};


BOOST_FIXTURE_TEST_SUITE(recent_file_list_test, recent_file_list_test_fixture)


/// Tests initialization
BOOST_AUTO_TEST_CASE(init) {
    BOOST_CHECK(lst.files().empty());
}


/// Tests adding new files
BOOST_AUTO_TEST_CASE(add_new) {
    lst.add(L"/home/user/file1");

    BOOST_REQUIRE_EQUAL(lst.files().size(), 1);
    BOOST_CHECK(*std::ranges::begin(lst.files()) == L"/home/user/file1");

    auto saved_files = sett.read<std::vector<std::filesystem::path>>("mylist", {});
    BOOST_REQUIRE_EQUAL(saved_files.size(), 1);
    BOOST_CHECK(saved_files[0] == L"/home/user/file1");

    lst.add(L"/home/user/file2");

    BOOST_REQUIRE_EQUAL(lst.files().size(), 2);
    auto files = lst.files();
    auto it = std::ranges::begin(files);
    BOOST_CHECK(*it == L"/home/user/file2");
    ++it;
    BOOST_CHECK(*it == L"/home/user/file1");

    auto saved_files2 = sett.read<std::vector<std::filesystem::path>>("mylist", {});
    BOOST_REQUIRE_EQUAL(saved_files2.size(), 2);
    BOOST_CHECK(saved_files2[0] == L"/home/user/file2");
    BOOST_CHECK(saved_files2[1] == L"/home/user/file1");
}


/// Tests adding existing file
BOOST_AUTO_TEST_CASE(add_existing) {
    lst.add(L"/home/user/file1");
    lst.add(L"/home/user/file2");
    lst.add(L"/home/user/file1");

    BOOST_REQUIRE_EQUAL(lst.files().size(), 2);
    auto files = lst.files();
    auto it = std::ranges::begin(files);
    BOOST_CHECK(*it == L"/home/user/file1");
    ++it;
    BOOST_CHECK(*it == L"/home/user/file2");

    auto saved_files = sett.read<std::vector<std::filesystem::path>>("mylist", {});
    BOOST_REQUIRE_EQUAL(saved_files.size(), 2);
    BOOST_CHECK(saved_files[0] == L"/home/user/file1");
    BOOST_CHECK(saved_files[1] == L"/home/user/file2");
}


/// Tests removing old files
BOOST_AUTO_TEST_CASE(add_remove_old) {
    lst.add(L"/home/user/file1");
    lst.add(L"/home/user/file2");
    lst.add(L"/home/user/file3");
    lst.add(L"/home/user/file4");
    lst.add(L"/home/user/file5");
    lst.add(L"/home/user/file6");

    BOOST_REQUIRE_EQUAL(lst.files().size(), 5);
    auto files = lst.files();
    auto it = std::ranges::begin(files);
    BOOST_CHECK(*it == L"/home/user/file6");
    ++it;
    BOOST_CHECK(*it == L"/home/user/file5");
    ++it;
    BOOST_CHECK(*it == L"/home/user/file4");
    ++it;
    BOOST_CHECK(*it == L"/home/user/file3");
    ++it;
    BOOST_CHECK(*it == L"/home/user/file2");


    auto saved_files = sett.read<std::vector<std::filesystem::path>>("mylist", {});
    BOOST_REQUIRE_EQUAL(saved_files.size(), 5);
    BOOST_CHECK(saved_files[0] == L"/home/user/file6");
    BOOST_CHECK(saved_files[1] == L"/home/user/file5");
    BOOST_CHECK(saved_files[2] == L"/home/user/file4");
    BOOST_CHECK(saved_files[3] == L"/home/user/file3");
    BOOST_CHECK(saved_files[4] == L"/home/user/file2");
}


/// Tests adding existing file into full list
BOOST_AUTO_TEST_CASE(add_existing_full) {
    lst.add(L"/home/user/file1");
    lst.add(L"/home/user/file2");
    lst.add(L"/home/user/file3");
    lst.add(L"/home/user/file4");
    lst.add(L"/home/user/file5");
    lst.add(L"/home/user/file3");

    BOOST_REQUIRE_EQUAL(lst.files().size(), 5);
    auto files = lst.files();
    auto it = std::ranges::begin(files);
    BOOST_CHECK(*it == L"/home/user/file3");
    ++it;
    BOOST_CHECK(*it == L"/home/user/file5");
    ++it;
    BOOST_CHECK(*it == L"/home/user/file4");
    ++it;
    BOOST_CHECK(*it == L"/home/user/file2");
    ++it;
    BOOST_CHECK(*it == L"/home/user/file1");


    auto saved_files = sett.read<std::vector<std::filesystem::path>>("mylist", {});
    BOOST_REQUIRE_EQUAL(saved_files.size(), 5);
    BOOST_CHECK(saved_files[0] == L"/home/user/file3");
    BOOST_CHECK(saved_files[1] == L"/home/user/file5");
    BOOST_CHECK(saved_files[2] == L"/home/user/file4");
    BOOST_CHECK(saved_files[3] == L"/home/user/file2");
    BOOST_CHECK(saved_files[4] == L"/home/user/file1");
}



/// Tests clearing list
BOOST_AUTO_TEST_CASE(clear) {
    lst.add(L"/home/user/file1");
    lst.add(L"/home/user/file2");
    lst.add(L"/home/user/file3");
    lst.add(L"/home/user/file4");
    lst.clear();

    BOOST_CHECK_EQUAL(lst.files().size(), 0);

    auto saved_files = sett.read<std::vector<std::filesystem::path>>("mylist", {});
    BOOST_CHECK_EQUAL(saved_files.size(), 0);
}


/// Tests loading list
BOOST_AUTO_TEST_CASE(save_load) {
    lst.add(L"/home/user/file1");
    lst.add(L"/home/user/file2");
    lst.add(L"/home/user/sss");
    lst.add(L"/home/user/file4");

    recent_file_list lst2{sett, "mylist", 5};

    BOOST_REQUIRE_EQUAL(lst2.files().size(), 4);
    auto files = lst2.files();
    auto it = std::ranges::begin(files);
    BOOST_CHECK(*it == L"/home/user/file4");
    ++it;
    BOOST_CHECK(*it == L"/home/user/sss");
    ++it;
    BOOST_CHECK(*it == L"/home/user/file2");
    ++it;
    BOOST_CHECK(*it == L"/home/user/file1");
}



BOOST_AUTO_TEST_SUITE_END()


}
