// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file async_data_fetcher_test.cpp
/// Contains unit tests for the async_data_fetcher class.

#include "mock_async_data_source.hpp"
#include "../async_data_fetcher.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::test {


struct async_data_fetcher_test_fixture {
    mock_async_data_source data_src;
    async_data_fetcher fetcher{data_src};
    const vector_model<char> & data = fetcher.data();

    /// Returns string created from data
    std::string data_str() const {
        return std::string{data.begin(), data.end()};
    }
};


/// Converts string to vector of characters
std::vector<char> str_to_vector(const std::string & s) {
    std::vector<char> res;
    res.reserve(s.size());
    std::copy(s.begin(), s.end(), std::back_inserter(res));
    return res;
}


BOOST_FIXTURE_TEST_SUITE(async_data_fetcher_test, async_data_fetcher_test_fixture)


/// Tests constructor
BOOST_AUTO_TEST_CASE(ctor) {
    BOOST_CHECK_EQUAL(fetcher.start_addr(), UINT64_MAX);
    BOOST_CHECK_EQUAL(data.size(), 0);
}


/// Tests fetching data after reset
BOOST_AUTO_TEST_CASE(reset_fetch) {
    MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, 1000);
        BOOST_CHECK_EQUAL(size, 10);
        handler(std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'x', 'x'});
    });

    fetcher.reset(1000, 10);

    BOOST_REQUIRE_EQUAL(data.size(), 10);
    std::string data_str{data.begin(), data.end()};
    BOOST_CHECK_EQUAL(data_str, "abcdefghxx");

    BOOST_CHECK(data_src.verify());
}


/// Tests calling reset twice
BOOST_AUTO_TEST_CASE(reset_twice) {
    async_data_source::completion_handler first_handler;
    MOCK_ADD_CALL(data_src, read_data, [&first_handler](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, 1000);
        BOOST_CHECK_EQUAL(size, 10);
        first_handler = handler;
    });

    fetcher.reset(1000, 10);

    BOOST_CHECK_EQUAL(data.size(), 0);

    async_data_source::completion_handler second_handler;
    MOCK_ADD_CALL(data_src, read_data, [&second_handler](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, 2000);
        BOOST_CHECK_EQUAL(size, 5);
        second_handler = handler;
    });

    fetcher.reset(2000, 5);

    BOOST_CHECK_EQUAL(data.size(), 0);

    std::string first_data_str = "abcdefghxx";
    first_handler(str_to_vector(first_data_str));
    BOOST_CHECK_EQUAL(fetcher.start_addr(), UINT64_MAX);
    BOOST_CHECK_EQUAL(data.size(), 0);

    std::string second_data_str = "12345";
    second_handler(str_to_vector(second_data_str));
    BOOST_CHECK_EQUAL(fetcher.start_addr(), 2000);
    BOOST_CHECK_EQUAL(data_str(), second_data_str);

    BOOST_CHECK(data_src.verify());
}


/// Tests fetching data before the beginning of fetched region
BOOST_AUTO_TEST_CASE(fetch_begin) {
    MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, 1000);
        BOOST_CHECK_EQUAL(size, 10);
        handler(std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'x', 'x'});
    });
    fetcher.reset(1000, 10);

    async_data_source::completion_handler handler;
    MOCK_ADD_CALL(data_src, read_data, [&handler](auto start, auto size, auto h) {
        BOOST_CHECK_EQUAL(start, 995);
        BOOST_CHECK_EQUAL(size, 5);
        handler = h;
    });

    fetcher.fetch_begin(5);

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");

    handler(std::vector<char>{'1', '2', '3', '4', '5'});

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 995);
    BOOST_CHECK_EQUAL(data_str(), "12345abcdefghxx");

    BOOST_CHECK(data_src.verify());
}


/// Tests fetching data before the zero address
BOOST_AUTO_TEST_CASE(fetch_begin_zero) {
    MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, 0);
        BOOST_CHECK_EQUAL(size, 10);
        handler(std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'x', 'x'});
    });
    fetcher.reset(0, 10);

    fetcher.fetch_begin(5);

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 0);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");

    BOOST_CHECK(data_src.verify());
}


/// Tests fetching data before the beginning of fetched region near the zero offset
BOOST_AUTO_TEST_CASE(fetch_begin_near_zero) {
    MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, 5);
        BOOST_CHECK_EQUAL(size, 10);
        handler(std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'x', 'x'});
    });
    fetcher.reset(5, 10);

    async_data_source::completion_handler handler;
    MOCK_ADD_CALL(data_src, read_data, [&handler](auto start, auto size, auto h) {
        BOOST_CHECK_EQUAL(start, 0);
        BOOST_CHECK_EQUAL(size, 5);
        handler = h;
    });

    fetcher.fetch_begin(10);

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 5);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");

    handler(std::vector<char>{'1', '2', '3', '4', '5'});

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 0);
    BOOST_CHECK_EQUAL(data_str(), "12345abcdefghxx");

    BOOST_CHECK(data_src.verify());
}


/// Tests fetching data before the beginning of fetched region twice
BOOST_AUTO_TEST_CASE(fetch_begin_twice) {
    MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, 1000);
        BOOST_CHECK_EQUAL(size, 10);
        handler(std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'x', 'x'});
    });
    fetcher.reset(1000, 10);

    async_data_source::completion_handler handler1;
    MOCK_ADD_CALL(data_src, read_data, [&handler1](auto start, auto size, auto h) {
        BOOST_CHECK_EQUAL(start, 995);
        BOOST_CHECK_EQUAL(size, 5);
        handler1 = h;
    });

    fetcher.fetch_begin(5);

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");

    async_data_source::completion_handler handler2;
    MOCK_ADD_CALL(data_src, read_data, [&handler2](auto start, auto size, auto h) {
        BOOST_CHECK_EQUAL(start, 990);
        BOOST_CHECK_EQUAL(size, 5);
        handler2 = h;
    });

    fetcher.fetch_begin(10);

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");


    handler1(std::vector<char>{'6', '7', '8', '9', '0'});

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 995);
    BOOST_CHECK_EQUAL(data_str(), "67890abcdefghxx");


    handler2(std::vector<char>{'1', '2', '3', '4', '5'});

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 990);
    BOOST_CHECK_EQUAL(data_str(), "1234567890abcdefghxx");

    BOOST_CHECK(data_src.verify());
}


/// Tests resetting after fetching data at beginning
BOOST_AUTO_TEST_CASE(reset_after_fetch_begin) {
    // resetting with initial data
    MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, 1000);
        BOOST_CHECK_EQUAL(size, 10);
        handler(std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'x', 'x'});
    });
    fetcher.reset(1000, 10);


    // fetching before reset

    async_data_source::completion_handler fetch_handler;
    MOCK_ADD_CALL(data_src, read_data, [&fetch_handler](auto start, auto size, auto h) {
        BOOST_CHECK_EQUAL(start, 995);
        BOOST_CHECK_EQUAL(size, 5);
        fetch_handler = h;
    });

    fetcher.fetch_begin(5);

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");


    // resetting before fetch_begin handler is called

    async_data_source::completion_handler reset_handler;
    MOCK_ADD_CALL(data_src, read_data, [&reset_handler](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, 2000);
        BOOST_CHECK_EQUAL(size, 20);
        reset_handler = handler;
    });

    fetcher.reset(2000, 20);

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");


    // calling fetch handler

    fetch_handler(std::vector<char>{'1', '2', '3', '4', '5'});

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");


    // calling reset handler

    std::string reset_str = "1234567890abcdefghxx";
    reset_handler(std::vector<char>{reset_str.begin(), reset_str.end()});

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 2000);
    BOOST_CHECK_EQUAL(data_str(), reset_str);


    BOOST_CHECK(data_src.verify());
}


/// Tests resetting before fetching data at beginning
BOOST_AUTO_TEST_CASE(reset_before_fetch_begin) {
    // resetting with initial data
    MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, 1000);
        BOOST_CHECK_EQUAL(size, 10);
        handler(std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'x', 'x'});
    });
    fetcher.reset(1000, 10);


    // resetting before fetch_begin

    async_data_source::completion_handler reset_handler;
    MOCK_ADD_CALL(data_src, read_data, [&reset_handler](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, 2000);
        BOOST_CHECK_EQUAL(size, 20);
        reset_handler = handler;
    });

    fetcher.reset(2000, 20);

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");


    // fetching after resetting

    fetcher.fetch_begin(5);

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");


    // calling reset handler

    std::string reset_str = "1234567890abcdefghxx";
    reset_handler(std::vector<char>{reset_str.begin(), reset_str.end()});

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 2000);
    BOOST_CHECK_EQUAL(data_str(), reset_str);

    BOOST_CHECK(data_src.verify());
}


/// Tests fetching data after the end of fetched region
BOOST_AUTO_TEST_CASE(fetch_end) {
    MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, 1000);
        BOOST_CHECK_EQUAL(size, 10);
        handler(std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'x', 'x'});
    });
    fetcher.reset(1000, 10);

    async_data_source::completion_handler handler;
    MOCK_ADD_CALL(data_src, read_data, [&handler](auto start, auto size, auto h) {
        BOOST_CHECK_EQUAL(start, 1010);
        BOOST_CHECK_EQUAL(size, 5);
        handler = h;
    });

    fetcher.fetch_end(5);

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");

    handler(std::vector<char>{'1', '2', '3', '4', '5'});

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx12345");

    BOOST_CHECK(data_src.verify());
}


/// Tests fetching data after the max address
BOOST_AUTO_TEST_CASE(fetch_end_max) {
    MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, UINT64_MAX - 10);
        BOOST_CHECK_EQUAL(size, 10);
        handler(std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'x', 'x'});
    });
    fetcher.reset(UINT64_MAX - 10, 10);

    fetcher.fetch_end(5);

    BOOST_CHECK_EQUAL(fetcher.start_addr(), UINT64_MAX - 10);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");

    BOOST_CHECK(data_src.verify());
}


/// Tests fetching data after the end of fetched region near the max offset
BOOST_AUTO_TEST_CASE(fetch_end_near_max) {
    MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, UINT64_MAX - 15);
        BOOST_CHECK_EQUAL(size, 10);
        handler(std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'x', 'x'});
    });
    fetcher.reset(UINT64_MAX - 15, 10);

    async_data_source::completion_handler handler;
    MOCK_ADD_CALL(data_src, read_data, [&handler](auto start, auto size, auto h) {
        BOOST_CHECK_EQUAL(start, UINT64_MAX - 5);
        BOOST_CHECK_EQUAL(size, 5);
        handler = h;
    });

    fetcher.fetch_end(10);

    BOOST_CHECK_EQUAL(fetcher.start_addr(), UINT64_MAX - 15);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");

    handler(std::vector<char>{'1', '2', '3', '4', '5'});

    BOOST_CHECK_EQUAL(fetcher.start_addr(), UINT64_MAX - 15);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx12345");

    BOOST_CHECK(data_src.verify());
}


/// Tests fetching data after the end of fetched region twice
BOOST_AUTO_TEST_CASE(fetch_end_twice) {
    MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, 1000);
        BOOST_CHECK_EQUAL(size, 10);
        handler(std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'x', 'x'});
    });
    fetcher.reset(1000, 10);

    async_data_source::completion_handler handler1;
    MOCK_ADD_CALL(data_src, read_data, [&handler1](auto start, auto size, auto h) {
        BOOST_CHECK_EQUAL(start, 1010);
        BOOST_CHECK_EQUAL(size, 10);
        handler1 = h;
    });

    fetcher.fetch_end(10);

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");

    fetcher.fetch_end(5);

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");

    handler1(std::vector<char>{'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'});

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx1234567890");

    BOOST_CHECK(data_src.verify());
}


/// Tests resetting after fetching data at end
BOOST_AUTO_TEST_CASE(reset_after_fetch_end) {
    // resetting with initial data
    MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, 1000);
        BOOST_CHECK_EQUAL(size, 10);
        handler(std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'x', 'x'});
    });
    fetcher.reset(1000, 10);


    // fetching before reset

    async_data_source::completion_handler fetch_handler;
    MOCK_ADD_CALL(data_src, read_data, [&fetch_handler](auto start, auto size, auto h) {
        BOOST_CHECK_EQUAL(start, 1010);
        BOOST_CHECK_EQUAL(size, 5);
        fetch_handler = h;
    });

    fetcher.fetch_end(5);

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");


    // resetting before fetch handler is called

    async_data_source::completion_handler reset_handler;
    MOCK_ADD_CALL(data_src, read_data, [&reset_handler](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, 2000);
        BOOST_CHECK_EQUAL(size, 20);
        reset_handler = handler;
    });

    fetcher.reset(2000, 20);

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");


    // calling fetch handler

    fetch_handler(std::vector<char>{'1', '2', '3', '4', '5'});

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");


    // calling reset handler

    std::string reset_str = "1234567890abcdefghxx";
    reset_handler(std::vector<char>{reset_str.begin(), reset_str.end()});

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 2000);
    BOOST_CHECK_EQUAL(data_str(), reset_str);

    BOOST_CHECK(data_src.verify());
}


/// Tests resetting before fetching data at end
BOOST_AUTO_TEST_CASE(reset_before_fetch_end) {
    // resetting with initial data
    MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, 1000);
        BOOST_CHECK_EQUAL(size, 10);
        handler(std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'x', 'x'});
    });
    fetcher.reset(1000, 10);


    // resetting before fetch_begin

    async_data_source::completion_handler reset_handler;
    MOCK_ADD_CALL(data_src, read_data, [&reset_handler](auto start, auto size, auto handler) {
        BOOST_CHECK_EQUAL(start, 2000);
        BOOST_CHECK_EQUAL(size, 20);
        reset_handler = handler;
    });

    fetcher.reset(2000, 20);

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");


    // fetching after resetting

    fetcher.fetch_end(5);

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
    BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");


    // calling reset handler

    std::string reset_str = "1234567890abcdefghxx";
    reset_handler(std::vector<char>{reset_str.begin(), reset_str.end()});

    BOOST_CHECK_EQUAL(fetcher.start_addr(), 2000);
    BOOST_CHECK_EQUAL(data_str(), reset_str);

    BOOST_CHECK(data_src.verify());
}


// /// Tests fetching data with reset after it becomes available
// BOOST_AUTO_TEST_CASE(reset_fetch_data_after_available) {
//     data_src.set_state(async_data_source_sate::unavailable);

//     // calling reset. Fetches should save start address and size,
//     // but should not fetch data
//     fetcher.reset(1000, 10);

//     BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
//     BOOST_CHECK_EQUAL(data.size(), 0);

//     // changing state to updating should not cause any changes
//     data_src.set_state(async_data_source_sate::updating);

//     // fetcher should read data after state changes to available
//     MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
//         BOOST_CHECK_EQUAL(start, 1000);
//         BOOST_CHECK_EQUAL(size, 10);
//         handler(std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'x', 'x'});
//     });

//     data_src.set_state(async_data_source_sate::available);

//     BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");

//     BOOST_CHECK(data_src.verify());
// }


// /// Tests calling reset multiple times when data is unavailable
// BOOST_AUTO_TEST_CASE(reset_twice_unavailable) {
//     data_src.set_state(async_data_source_sate::unavailable);

//     // calling reset. Fetches should save start address and size,
//     // but should not fetch data
//     fetcher.reset(1000, 5);

//     BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
//     BOOST_CHECK_EQUAL(data.size(), 0);

//     // calling reset second time. Fetches should save new address and size,
//     // but should not fetch data
//     fetcher.reset(2000, 10);

//     // fetcher should read data after state changes to available
//     MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
//         BOOST_CHECK_EQUAL(start, 2000);
//         BOOST_CHECK_EQUAL(size, 10);
//         handler(std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'x', 'x'});
//     });

//     data_src.set_state(async_data_source_sate::available);

//     BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");

//     BOOST_CHECK(data_src.verify());
// }


// /// Tests fetching data after it becomes available with reset called befor unavailable state
// BOOST_AUTO_TEST_CASE(reset_before_unavailable_fetch_data_after_available) {
//     // fetcher should read data after state changes to available
//     MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
//         BOOST_CHECK_EQUAL(start, 1000);
//         BOOST_CHECK_EQUAL(size, 10);
//         handler(std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'x', 'x'});
//     });

//     // calling reset. Fetches should fetch data
//     fetcher.reset(1000, 10);

//     BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
//     BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");

//     // after setting state to unavailable, fetcher should clear all data
//     data_src.set_state(async_data_source_sate::unavailable);
//     BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
//     BOOST_CHECK_EQUAL(data.size(), 0);

//     // changing state to updating should not cause any changes
//     data_src.set_state(async_data_source_sate::updating);

//     // fetcher should read data after state changes to available

//     MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
//         BOOST_CHECK_EQUAL(start, 1000);
//         BOOST_CHECK_EQUAL(size, 10);
//         handler(std::vector<char>{'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'});
//     });

//     data_src.set_state(async_data_source_sate::available);

//     BOOST_CHECK_EQUAL(data_str(), "1234567890");

//     BOOST_CHECK(data_src.verify());
// }


// /// Tests ignoring fetch_begin call when data is not available
// BOOST_AUTO_TEST_CASE(unavailable_ignore_fetch_begin) {
//     data_src.set_state(async_data_source_sate::unavailable);

//     fetcher.reset(1000, 5);

//     BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
//     BOOST_CHECK_EQUAL(data.size(), 0);

//     fetcher.fetch_begin(10);

//     BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
//     BOOST_CHECK_EQUAL(data.size(), 0);

//     // fetcher should read data after state changes to available

//     MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
//         BOOST_CHECK_EQUAL(start, 2000);
//         BOOST_CHECK_EQUAL(size, 10);
//         handler(std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'x', 'x'});
//     });

//     data_src.set_state(async_data_source_sate::available);

//     BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");

//     BOOST_CHECK(data_src.verify());
// }


// /// Tests ignoring fetch_end call when data is not available
// BOOST_AUTO_TEST_CASE(unavailable_ignore_fetch_end) {
//     data_src.set_state(async_data_source_sate::unavailable);

//     fetcher.reset(1000, 5);

//     BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
//     BOOST_CHECK_EQUAL(data.size(), 0);

//     fetcher.fetch_end(10);

//     BOOST_CHECK_EQUAL(fetcher.start_addr(), 1000);
//     BOOST_CHECK_EQUAL(data.size(), 0);

//     // fetcher should read data after state changes to available

//     MOCK_ADD_CALL(data_src, read_data, [](auto start, auto size, auto handler) {
//         BOOST_CHECK_EQUAL(start, 2000);
//         BOOST_CHECK_EQUAL(size, 10);
//         handler(std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'x', 'x'});
//     });

//     data_src.set_state(async_data_source_sate::available);

//     BOOST_CHECK_EQUAL(data_str(), "abcdefghxx");

//     BOOST_CHECK(data_src.verify());
// }


BOOST_AUTO_TEST_SUITE_END()


}
