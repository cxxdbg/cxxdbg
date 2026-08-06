// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file result_test.cpp
/// Contains unit tests for result class.

#include "cxxdbg/async/result.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg { namespace async { namespace test {


BOOST_AUTO_TEST_SUITE(result_test)


/// Tests void initialization with ok
BOOST_AUTO_TEST_CASE(void_init_ok) {
    async::result<> res;
    BOOST_CHECK(res.is_ok());
}


/// Tests void initialization with error
BOOST_AUTO_TEST_CASE(void_init_error) {
    async::result<> res{"myerr"};
    BOOST_CHECK(!res.is_ok());
    BOOST_CHECK(res.error() == "myerr");
}


/// Tests setting ok status on ok void result
BOOST_AUTO_TEST_CASE(void_ok_set_ok) {
    async::result<> res;
    res.set_ok();

    BOOST_CHECK(res.is_ok());
}


/// Tests setting ok status on error void result
BOOST_AUTO_TEST_CASE(void_error_set_ok) {
    async::result<> res{"errr"};
    res.set_ok();

    BOOST_CHECK(res.is_ok());
}


/// Tests setting error status on error void result
BOOST_AUTO_TEST_CASE(void_error_set_error) {
    async::result<> res{"er"};
    res.set_error("err2");

    BOOST_CHECK(!res.is_ok());
    BOOST_CHECK(res.error() == "err2");
}


/// Tests setting error status on ok void result
BOOST_AUTO_TEST_CASE(void_ok_set_error) {
    async::result<> res;
    res.set_error("err2");

    BOOST_CHECK(!res.is_ok());
    BOOST_CHECK(res.error() == "err2");
}


/// Tests copy constructor from ok int to void result
BOOST_AUTO_TEST_CASE(void_test_copy_int_ok) {
    async::result<int> r{11};
    async::result<> vr{r};

    BOOST_CHECK(vr.is_ok());
}


/// Tests copy constructor from error int to void result
BOOST_AUTO_TEST_CASE(void_test_copy_int_error) {
    async::result<int> r{"my err"};
    async::result<> vr{r};

    BOOST_CHECK(!vr.is_ok());
    BOOST_CHECK(vr.error() == "my err");
}


/// Tests assignment operator from ok int to ok void result
BOOST_AUTO_TEST_CASE(void_test_assign_int_ok_ok) {
    async::result<int> r{11};
    async::result<> vr;
    vr = r;

    BOOST_CHECK(vr.is_ok());
}

/// Tests assignment operator from ok int to error void result
BOOST_AUTO_TEST_CASE(void_test_assign_int_ok_error) {
    async::result<int> r{11};
    async::result<> vr{"err"};
    vr = r;

    BOOST_CHECK(vr.is_ok());
}


/// Tests assignment operator from error int to ok void result
BOOST_AUTO_TEST_CASE(void_test_assign_int_error_ok) {
    async::result<int> r{"11"};
    async::result<> vr;
    vr = r;

    BOOST_CHECK(!vr.is_ok());
    BOOST_CHECK(vr.error() == "11");
}

/// Tests assignment operator from error int to error void result
BOOST_AUTO_TEST_CASE(void_test_assign_int_error_error) {
    async::result<int> r{"12"};
    async::result<> vr{"err"};
    vr = r;

    BOOST_CHECK(!vr.is_ok());
    BOOST_CHECK(vr.error() == "12");
}


/// Tests conversion from ok void result to int
BOOST_AUTO_TEST_CASE(void_to_int_ok) {
    async::result<> r;
    auto r2 = r.to<int>(10);

    BOOST_CHECK(r2.is_ok());
    BOOST_CHECK(r2.value() == 10);
}



/// Tests int initialization with ok
BOOST_AUTO_TEST_CASE(int_init_ok) {
    async::result<int> res{20};
    BOOST_CHECK(res.is_ok());
    BOOST_CHECK(res.value() == 20);
}


/// Tests int initialization with error
BOOST_AUTO_TEST_CASE(int_init_error) {
    async::result<int> res{"myerr"};
    BOOST_CHECK(!res.is_ok());
    BOOST_CHECK(res.error() == "myerr");
}


/// Tests setting ok status on ok int result
BOOST_AUTO_TEST_CASE(int_ok_set_ok) {
    async::result<int> res{20};
    res.set_value(30);

    BOOST_CHECK(res.is_ok());
    BOOST_CHECK(res.value() == 30);
}


/// Tests setting ok status on error int result
BOOST_AUTO_TEST_CASE(int_error_set_ok) {
    async::result<int> res{"errr"};
    res.set_value(20);

    BOOST_CHECK(res.is_ok());
    BOOST_CHECK(res.value() == 20);
}


/// Tests setting error status on error int result
BOOST_AUTO_TEST_CASE(int_error_set_error) {
    async::result<int> res{"er"};
    res.set_error("err2");

    BOOST_CHECK(!res.is_ok());
    BOOST_CHECK(res.error() == "err2");
}


/// Tests setting error status on ok int result
BOOST_AUTO_TEST_CASE(int_ok_set_error) {
    async::result<int> res{20};
    res.set_error("err2");

    BOOST_CHECK(!res.is_ok());
    BOOST_CHECK(res.error() == "err2");
}


/// Tests copy constructor from long ok to int result
BOOST_AUTO_TEST_CASE(copy_long_int_ok) {
    async::result<long> r1{44};
    async::result<int> r2{r1};

    BOOST_CHECK(r2.is_ok());
    BOOST_CHECK(r2.value() == 44);
}


/// Tests copy constructor from long error to int result
BOOST_AUTO_TEST_CASE(copy_long_int_error) {
    async::result<long> r1{"aaa"};
    async::result<int> r2{r1};

    BOOST_CHECK(!r2.is_ok());
    BOOST_CHECK(r2.error() == "aaa");
}


/// Tests assignment from long ok to int ok result
BOOST_AUTO_TEST_CASE(assign_long_int_ok_ok) {
    async::result<long> r1{44};
    async::result<int> r2{12};
    r2 = r1;

    BOOST_CHECK(r2.is_ok());
    BOOST_CHECK(r2.value() == 44);
}


/// Tests assignment from long ok to int error result
BOOST_AUTO_TEST_CASE(assign_long_int_ok_error) {
    async::result<long> r1{44};
    async::result<int> r2{"aaa"};
    r2 = r1;

    BOOST_CHECK(r2.is_ok());
    BOOST_CHECK(r2.value() == 44);
}


/// Tests assignment from long error to int ok result
BOOST_AUTO_TEST_CASE(assign_long_int_error_ok) {
    async::result<long> r1{"bbb"};
    async::result<int> r2{12};
    r2 = r1;

    BOOST_CHECK(!r2.is_ok());
    BOOST_CHECK(r2.error() == "bbb");
}


/// Tests assignment from long error to int error result
BOOST_AUTO_TEST_CASE(assign_long_int_error_error) {
    async::result<long> r1{"zzz"};
    async::result<int> r2{"aaa"};
    r2 = r1;

    BOOST_CHECK(!r2.is_ok());
    BOOST_CHECK(r2.error() == "zzz");
}


BOOST_AUTO_TEST_SUITE_END()


} } }
