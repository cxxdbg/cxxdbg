// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file test_watch_list_impl.cpp
/// Contains implementation of test_watch_list_impl class.

#include "test_watch_list_impl.hpp"


namespace cxxdbg::dbg::test {


test_watch_list_impl::~test_watch_list_impl() {
}


signal_connection test_watch_list_impl::connect_tree_updated(const tree_updated_handler & handl) {
    return tree_updated_signal_.connect(handl);
}


void test_watch_list_impl::emit_tree_updated_signal(const tree_info & tinfo) {
    tree_updated_signal_(tinfo);
}


}
