// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file memory_data_source.cpp
/// Contains definition of the memory_data_source class.

#include "memory_data_source.hpp"
#include "target.hpp"


namespace cxxdbg::dbg {


void memory_data_source::read_data(uint64_t offset, size_t size, const completion_handler & handler) const {
    targ_.read_memory(offset, size, [handler](auto && r) {
        if (r.is_ok()) {
            handler(r.value());
        } else {
            handler({});
        }
    });
}


}