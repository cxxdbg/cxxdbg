// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file async_data_fetcher.cpp
/// Contains implementation of the async_data_fetcher class.

#include "async_data_fetcher.hpp"


namespace cxxdbg {


async_data_fetcher::async_data_fetcher(async_data_source & src):
data_src_{src}, start_addr_{UINT64_MAX} {
}


async_data_fetcher::~async_data_fetcher() {
    assert(n_reset_ops_ == 0 && "not all reset operations finished");
    assert(n_fetch_begin_ops_ == 0 && "not all fetch begin operations finished");
    assert(n_fetch_end_ops_ == 0 && "not all fetch end operations finished");
    assert(n_skip_fetch_begin_ops_ == 0 && "not all fetch begin operations skipped");
    assert(n_skip_fetch_end_ops_ == 0 && "not all fetch end operations skipped");
}


void async_data_fetcher::reset(uint64_t s_addr, size_t size) {
    if (UINT64_MAX - size < s_addr) {
        size = UINT64_MAX - s_addr;
    }

    // ignoring all current fetch operations
    n_skip_fetch_begin_ops_ += n_fetch_begin_ops_;
    n_fetch_begin_ops_ = 0;
    fetch_begin_size_ = 0;
    n_skip_fetch_end_ops_ += n_fetch_end_ops_;
    n_fetch_end_ops_ = 0;
    fetch_end_size_ = 0;

    if (size == 0) {
        return;
    }

    ++n_reset_ops_;

    data_src_.read_data(s_addr, size, [this, s_addr, size](auto && data) {
        --n_reset_ops_;

        if (n_reset_ops_ == 0) {
            // this is the last reset operation in the chain, use its results
            assert(size == data.size() && "invalid data size read from source");
            start_addr_ = s_addr;
            data_.clear();
            data_.insert(0, data.begin(), data.end());
        } else {
            // another reset operation was started after that, ignore results
        }
    });
}


void async_data_fetcher::fetch_begin(size_t size) {
    // ignoring fetch request if reset operation is in progress
    if (n_reset_ops_ != 0) {
        return;
    }

    // calculating read start address and adjusting read size in case of beginning of address space
    auto read_size = size;
    uint64_t read_start_addr = 0;
    if (start_addr_ >= size) {
        read_start_addr = start_addr_ - read_size;
    } else {
        read_size = start_addr_;
        read_start_addr = 0;
    }

    if (read_size <= fetch_begin_size_) {
        // fetching of required number of bytes was already requested
        return; 
    }

    // adjusting read size taking into account read operations that are in progress
    read_size -= fetch_begin_size_;

    // increasing size of data begin read
    fetch_begin_size_ += read_size;

    ++n_fetch_begin_ops_;

    data_src_.read_data(read_start_addr, read_size, [this, read_size](auto && data) {
        assert(read_size == data.size() && "invalid data in read_data handler");

        if(n_skip_fetch_begin_ops_ != 0) {
            // skipping result of this operation
            --n_skip_fetch_begin_ops_;
            return;
        }

        assert(read_size <= fetch_begin_size_ && "invalid size in read_data handler");
        assert(start_addr_ >= read_size && "invalid start address in read_data handler");

        assert(n_fetch_begin_ops_ > 0 && "invalid number of fetch begin operations in progress");
        --n_fetch_begin_ops_;

        // adding received data into fetched data model and
        // adjusting fetch size and start address
        data_.insert(0, data.begin(), data.end());
        fetch_begin_size_ -= read_size;
        start_addr_ -= read_size;
    });
}


void async_data_fetcher::fetch_end(size_t size) {
    // ignoring fetch request if reset operation is in progress
    if (n_reset_ops_ != 0) {
        return;
    }

    // calculating read size in case of end of address space
    assert(start_addr_ <= SIZE_MAX - data_.size() && "invalid fetched data size");
    auto read_size = size;
    uint64_t read_start_addr = start_addr_ + data_.size();
    if (SIZE_MAX - read_size < read_start_addr) {
        read_size = SIZE_MAX - read_start_addr;
    }

    if (read_size <= fetch_end_size_) {
        // fetching of required number of bytes was already requested
        return;
    }

    // adjusting read size taking into account read operations that are in progress
    read_size -= fetch_end_size_;

    // increasing size of data end read
    fetch_end_size_ += read_size;

    ++n_fetch_end_ops_;

    data_src_.read_data(read_start_addr, read_size, [this, read_size](auto && data) {
        assert(read_size == data.size() && "invalid data in read_data handler");

        if(n_skip_fetch_end_ops_ != 0) {
            // skipping result of this operation
            --n_skip_fetch_end_ops_;
            return;
        }

        assert(read_size <= fetch_end_size_ && "invalid size in read_data handler");
        assert(start_addr_ >= read_size && "invalid start address in read_data handler");

        assert(n_fetch_end_ops_ > 0 && "invalid number of fetch end operations in progress");
        --n_fetch_end_ops_;

        // adding received data into fetched data model and
        // adjusting fetch size
        data_.insert(data_.size(), data.begin(), data.end());
        fetch_end_size_ -= read_size;
    });
}


}
