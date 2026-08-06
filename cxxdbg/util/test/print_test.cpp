// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "cxxdbg/util/print.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg { namespace util { namespace test {


BOOST_AUTO_TEST_SUITE(record_value_test)


/// Tests print_hex function
BOOST_AUTO_TEST_CASE(print_hex_simple) {
    std::ostringstream str;
    std::uint64_t val = 0x135;

    print_hex(str, val, 2, 2);
    std::string res = str.str();
    BOOST_CHECK(res == "0x0135");
}


/// Tests printing negative value as hex
BOOST_AUTO_TEST_CASE(print_hex_neg) {
    std::ostringstream str;
    std::int64_t val = -1;

    print_hex(str, val, 2, 2);
    std::string res = str.str();
    BOOST_CHECK(res == "0xFFFF");

}


/// Tests printing neg character
BOOST_AUTO_TEST_CASE(print_hex_char) {
    std::ostringstream str;
    char val = -1;

    print_hex(str, val, 1, 1);
    std::string res = str.str();
    BOOST_CHECK(res == "0xFF");
}


/// Tests printing neg character with no prefix
BOOST_AUTO_TEST_CASE(print_hex_char_no_prefix) {
    std::ostringstream str;
    char val = -1;

    print_hex_no_prefix(str, val, 1, 1);
    std::string res = str.str();
    BOOST_CHECK(res == "FF");
}


/// Tests parsing decimal uint64
BOOST_AUTO_TEST_CASE(parse_hex_uint64_dec) {
    std::string s{"2313"};

    std::uint64_t val;
    bool res = parse_hex_uint(s, val);

    BOOST_CHECK(res);
    BOOST_CHECK(val == 2313);
}


/// Tests parsing decimal uint64 with bad end
BOOST_AUTO_TEST_CASE(parse_hex_uint64_dec_bad_end) {
    std::string s{"2313Z"};

    std::uint64_t val;
    bool res = parse_hex_uint(s, val);

    BOOST_CHECK(!res);
}


/// Tests parsing hex uint64
BOOST_AUTO_TEST_CASE(parse_hex_uint64_hex) {
    std::string s{"0x005A5F"};

    std::uint64_t val;
    bool res = parse_hex_uint(s, val);

    BOOST_CHECK(res);
    BOOST_CHECK(val == 0x5A5F);
}


/// Tests parsing hex uint64 with bad end
BOOST_AUTO_TEST_CASE(parse_hex_uint64_hex_bad_end) {
    std::string s{"0x005A5FZ"};

    std::uint64_t val;
    bool res = parse_hex_uint(s, val);

    BOOST_CHECK(!res);
}


/// printing hex with minimum size
BOOST_AUTO_TEST_CASE(print_hex_min_size_8) {
    std::ostringstream str;
    uint64_t val = 0x2325;

    print_hex_no_prefix(str, val, 8, 8);
    std::string res = str.str();
    BOOST_CHECK_EQUAL(res, "0000000000002325");
}


/// printing hex with minimum size
BOOST_AUTO_TEST_CASE(print_hex_min_size_4) {
    std::ostringstream str;
    uint64_t val = 0x2325;

    print_hex_no_prefix(str, val, 8, 4);
    std::string res = str.str();
    BOOST_CHECK_EQUAL(res, "00002325");
}


/// printing hex with minimum size
BOOST_AUTO_TEST_CASE(print_hex_min_size_2) {
    std::ostringstream str;
    uint64_t val = 0x2325;

    print_hex_no_prefix(str, val, 8, 2);
    std::string res = str.str();
    BOOST_CHECK_EQUAL(res, "2325");
}


/// printing hex with minimum size
BOOST_AUTO_TEST_CASE(print_hex_min_size_1) {
    std::ostringstream str;
    uint64_t val = 0x2325;

    print_hex_no_prefix(str, val, 8, 1);
    std::string res = str.str();
    BOOST_CHECK_EQUAL(res, "0000000000002325");
}


/// printing hex with minimum size greather than size
BOOST_AUTO_TEST_CASE(print_hex_min_size_gt_size) {
    std::ostringstream str;
    uint64_t val = 0x2325;

    print_hex_no_prefix(str, val, 4, 8);
    std::string res = str.str();
    BOOST_CHECK_EQUAL(res, "00002325");
}


/// printing large hex with minimum size
BOOST_AUTO_TEST_CASE(print_large_hex_min_size) {
    std::ostringstream str;
    uint64_t val = 0x1BBAA2325;

    print_hex_no_prefix(str, val, 8, 4);
    std::string res = str.str();
    BOOST_CHECK_EQUAL(res, "00000001BBAA2325");
}



BOOST_AUTO_TEST_SUITE_END()


} } }
