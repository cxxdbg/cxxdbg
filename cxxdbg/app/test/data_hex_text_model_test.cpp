// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file data_hex_text_model_test.cpp
/// Contains unit tests for the data_hex_text_model class.

#include "../data_hex_text_model.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::test {


struct data_hex_text_model_test_fixture {
    vector_model<char> data;
    data_hex_text_model hex_mdl{data};
    const tmvc::wsimple_text_model & text{hex_mdl.text()};
};


BOOST_FIXTURE_TEST_SUITE(data_hex_text_model_test, data_hex_text_model_test_fixture)


/// Tests constructing empty model
BOOST_AUTO_TEST_CASE(construct_empty) {
    BOOST_CHECK_EQUAL(string(text).size(), 0);
}


/// Tests displaying single line binary data
BOOST_AUTO_TEST_CASE(display_single_line) {
    std::string str = "abcdefghxyz";
    data.insert(0, str.begin(), str.end());

    BOOST_REQUIRE_EQUAL(lines(text).size(), 1);
    BOOST_CHECK(line_str(text, 0) == L"61 62 63 64 65 66 67 68    78 79 7A");
}


/// Tests displaying multiple lines after adding more data at the end
BOOST_AUTO_TEST_CASE(multi_line_add_end) {
    std::string str = "abcdefghxyz";
    data.insert(0, str.begin(), str.end());

    BOOST_REQUIRE_EQUAL(lines(text).size(), 1);
    BOOST_CHECK(line_str(text, 0) == L"61 62 63 64 65 66 67 68    78 79 7A");

    std::string str2 = "0123456789";
    data.insert(data.size(), str2.begin(), str2.end());

    BOOST_REQUIRE_EQUAL(lines(text).size(), 2);
    BOOST_CHECK(line_str(text, 0) == L"61 62 63 64 65 66 67 68    78 79 7A 30 31 32 33 34");
    BOOST_CHECK(line_str(text, 1) == L"35 36 37 38 39");
}


/// Tests displaying multiple lines after adding data starting from new row
BOOST_AUTO_TEST_CASE(add_data_new_row) {
    std::string str = "abcdefghxyz01234";
    data.insert(0, str.begin(), str.end());

    BOOST_REQUIRE_EQUAL(lines(text).size(), 1);
    BOOST_CHECK(line_str(text, 0) == L"61 62 63 64 65 66 67 68    78 79 7A 30 31 32 33 34");

    std::string str2 = "56789";
    data.insert(data.size(), str2.begin(), str2.end());

    BOOST_REQUIRE_EQUAL(lines(text).size(), 2);
    BOOST_CHECK(line_str(text, 0) == L"61 62 63 64 65 66 67 68    78 79 7A 30 31 32 33 34");
    BOOST_CHECK(line_str(text, 1) == L"35 36 37 38 39");
}


/// Tests adding data at the beginning
BOOST_AUTO_TEST_CASE(add_begin) {
    std::string str = "abcdefghxyz0123456789";
    data.insert(0, str.begin(), str.end());

    BOOST_REQUIRE_EQUAL(lines(text).size(), 2);
    BOOST_CHECK(line_str(text, 0) == L"61 62 63 64 65 66 67 68    78 79 7A 30 31 32 33 34");
    BOOST_CHECK(line_str(text, 1) == L"35 36 37 38 39");

    // supported adding of data of size multiple to number of bytes in row
    std::string str2 = "ABCDEFGHIJKLMNOP";
    data.insert(0, str2.begin(), str2.end());

    BOOST_REQUIRE_EQUAL(lines(text).size(), 3);
    BOOST_CHECK(line_str(text, 0) == L"41 42 43 44 45 46 47 48    49 4A 4B 4C 4D 4E 4F 50");
    BOOST_CHECK(line_str(text, 1) == L"61 62 63 64 65 66 67 68    78 79 7A 30 31 32 33 34");
    BOOST_CHECK(line_str(text, 2) == L"35 36 37 38 39");
}


/// Tests changing data
BOOST_AUTO_TEST_CASE(change_data) {
    std::string str = "abcdefghxyz0123456789";
    data.insert(0, str.begin(), str.end());

    BOOST_REQUIRE_EQUAL(lines(text).size(), 2);
    BOOST_CHECK(line_str(text, 0) == L"61 62 63 64 65 66 67 68    78 79 7A 30 31 32 33 34");
    BOOST_CHECK(line_str(text, 1) == L"35 36 37 38 39");

    std::string str2 = "HXYZ";
    size_t str2_idx = 7;

    for (size_t i = 0, sz = str2.size(); i < sz; ++i) {
        data.set(str2_idx + i, str2[i]);
    }


    BOOST_REQUIRE_EQUAL(lines(text).size(), 2);
    BOOST_CHECK(line_str(text, 0) == L"61 62 63 64 65 66 67 48    58 59 5A 30 31 32 33 34");
    BOOST_CHECK(line_str(text, 1) == L"35 36 37 38 39");
}


BOOST_AUTO_TEST_SUITE_END()


}
