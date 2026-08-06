// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file settings_storage_test.cpp
/// Contains unit tests for the settings_storage class.

#include "../memory_settings_storage.hpp"
#include "cxxdbg/mock/mock.hpp"
#include "cxxdbg/app/settings_storage.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg {


namespace {
struct custom_class {
    custom_class(bool b = false): bad{b} {}
    bool bad;
};
}

template <>
struct settings_storage_traits<custom_class> {
    static std::string to_string(const custom_class & v) {
        return "custom_cls";
    }

    static bool from_string(const std::string & str, custom_class & v) {
        if (str == "custom_cls")
            return true;

        return false;
    }
};


}


namespace cxxdbg::test {


struct settings_storage_test_fixture {
    memory_settings_storage sett;
};


BOOST_FIXTURE_TEST_SUITE(settings_storage_test, settings_storage_test_fixture)


/// Tests reading default int from storage
BOOST_AUTO_TEST_CASE(read_def_int) {
    int res = sett.read<int>("name", 15);

    BOOST_CHECK(res == 15);
}


/// Tests reading int from storage
BOOST_AUTO_TEST_CASE(read_int) {
    sett.write_string("name", "2353");
    int res = sett.read<int>("name", 15);

    BOOST_CHECK(res == 2353);
}


/// Tests writing int to storage
BOOST_AUTO_TEST_CASE(write_int) {
    sett.write("x", 15);

    std::string val;
    bool res = sett.read_string("x", val);
    BOOST_CHECK(res);
    BOOST_CHECK(val == "15");
}


/// Tests writing custom class to storage
BOOST_AUTO_TEST_CASE(write_custom) {
    sett.write("var", custom_class{});

    std::string val;
    bool res = sett.read_string("var", val);
    BOOST_CHECK(res);
    BOOST_CHECK(val == "custom_cls");
}


/// Tests reading custom class from storage
BOOST_AUTO_TEST_CASE(read_custom_ok) {
    sett.write_string("var", "custom_cls");

    custom_class val = sett.read<custom_class>("var", custom_class{true});
    BOOST_CHECK(!val.bad);
}


/// Tests error reading custom class from storage
BOOST_AUTO_TEST_CASE(read_custom_error) {
    sett.write_string("var", "custsssss");

    custom_class val = sett.read<custom_class>("var", custom_class{true});
    BOOST_CHECK(val.bad);
}


/// Tests writing vector to settings
BOOST_AUTO_TEST_CASE(write_string_vector) {
    std::vector<std::string> vec{"aaa", "bbb", "ccc,aa", "dd\\dd"};
    sett.write("v", vec);

    std::string val;
    bool res = sett.read_string("v", val);
    BOOST_CHECK(res);
    BOOST_CHECK(val == ",aaa,bbb,ccc\\,aa,dd\\\\dd");
}


/// Tests reading empty vector from settings
BOOST_AUTO_TEST_CASE(read_empty_string_vector) {
    sett.write_string("v", "");

    std::vector<std::string> res = sett.read<std::vector<std::string>>("v", {});
    BOOST_CHECK(res.size() == 0);
}


/// Tests reading vector from settings
BOOST_AUTO_TEST_CASE(read_string_vector) {
    sett.write_string("v", ",aaa,bbb,ccc\\,aa,dd\\\\dd");

    std::vector<std::string> res = sett.read<std::vector<std::string>>("v", {});
    BOOST_REQUIRE(res.size() == 4);
    BOOST_CHECK(res[0] == "aaa");
    BOOST_CHECK(res[1] == "bbb");
    BOOST_CHECK(res[2] == "ccc,aa");
    BOOST_CHECK(res[3] == "dd\\dd");
}


/// Tests writing int vector to settings
BOOST_AUTO_TEST_CASE(write_int_vector) {
    std::vector<int> vec{10, 123, 22};
    sett.write("v", vec);

    std::string val;
    bool res = sett.read_string("v", val);
    BOOST_CHECK(res);
    BOOST_CHECK(val == ",10,123,22");
}


/// Tests reading int vector from settings
BOOST_AUTO_TEST_CASE(read_int_vector) {
    sett.write_string("v", ",10,123,22");

    std::vector<int> res = sett.read<std::vector<int>>("v", {});
    BOOST_REQUIRE(res.size() == 3);
    BOOST_CHECK(res[0] == 10);
    BOOST_CHECK(res[1] == 123);
    BOOST_CHECK(res[2] == 22);
}


/// Tests writing list to settings
BOOST_AUTO_TEST_CASE(write_string_list) {
    std::list<std::string> list{"aaa", "bbb", "ccc,aa", "dd\\dd"};
    sett.write("v", list);

    std::string val;
    bool res = sett.read_string("v", val);
    BOOST_CHECK(res);
    BOOST_CHECK(val == ",aaa,bbb,ccc\\,aa,dd\\\\dd");
}


/// Tests reading list from settings
BOOST_AUTO_TEST_CASE(read_string_list) {
    sett.write_string("v", ",aaa,bbb,ccc\\,aa,dd\\\\dd");

    std::list<std::string> res = sett.read<std::list<std::string>>("v", {});
    BOOST_REQUIRE(res.size() == 4);

    auto it = std::begin(res);
    BOOST_REQUIRE(it != std::end(res));
    BOOST_CHECK(*it == "aaa");

    ++it;
    BOOST_REQUIRE(it != std::end(res));
    BOOST_CHECK(*it == "bbb");

    ++it;
    BOOST_REQUIRE(it != std::end(res));
    BOOST_CHECK(*it == "ccc,aa");

    ++it;
    BOOST_REQUIRE(it != std::end(res));
    BOOST_CHECK(*it == "dd\\dd");

    ++it;
    BOOST_CHECK(it == std::end(res));
}


BOOST_AUTO_TEST_SUITE_END()


}
