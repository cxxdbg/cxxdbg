// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file mock_async_data_source.hpp
/// Contains definition of the mock_async_data_source class.

#pragma once

#include "../async_data_source.hpp"
#include <cassert>
#include "cxxdbg/mock/mock.hpp"


namespace cxxdbg::test {


/// Mock implementation for the async_data_source interface
class mock_async_data_source: public mock::object<mock_async_data_source>, public async_data_source {
public:
    mock_async_data_source(size_t sz = SIZE_MAX, state_t st = state_t::available):
        size_{sz}, state_{st} {}

    void set_size(size_t sz) {
        size_ = sz;
    }

    uint64_t size() const override {
        return size_;
    }

    MOCK_DEFINE_METHOD_3_C(read_data, void (uint64_t, size_t, const completion_handler &))

    state_t state() const override {
        return state_;
    }

    void set_state(state_t st) {
        if (state_ == st) {
            return;
        }

        state_ = st;
        state_changed();
    }

private:
    size_t size_;
    state_t state_;
};

}
