// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file test_watch_list_impl.hpp
/// Contains definition of test_watch_list_impl class.

#pragma once

#include "cxxdbg/dbg/watch_list_impl.hpp"
#include "cxxdbg/app/signals.hpp"
#include "cxxdbg/mock/mock.hpp"


namespace cxxdbg::dbg::test {


/// \class test_watch_list_impl
/// Simple implementation of watch_list_impl interface for testing
class test_watch_list_impl:
        virtual public watch_list_impl,
        public mock::object<test_watch_list_impl> {

public:
    /// Destructor, destroys object
    virtual ~test_watch_list_impl();

    /// Connects to tree updated signal.
    signal_connection connect_tree_updated(const tree_updated_handler & handl) override;

    /// Emits tree updated signal with specified tree info
    void emit_tree_updated_signal(const tree_info & tinfo);

    MOCK_DEFINE_METHOD_2(expand_node, void(watch_list_impl::node_id, const node_handler &))
    MOCK_DEFINE_METHOD_2(collapse_node, void(watch_list_impl::node_id, const node_handler & handl))
    MOCK_DEFINE_METHOD_2(set_fmt_opts, void(const dbgfmt::format_options & fmt_opts, const tree_updated_handler & handl))

private:
    signal<tree_updated_func> tree_updated_signal_;
};


}


