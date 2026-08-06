// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file mock_custom_watch_list_impl.hpp
/// Contains definition of the mock_custom_watch_list_impl class.

#pragma once

#include "test_watch_list_impl.hpp"
#include "cxxdbg/dbg/custom_watch_list_impl.hpp"
#include "cxxdbg/mock/mock.hpp"


namespace cxxdbg::dbg::test {


/// Mock class for custom_watch_list_impl interface
class mock_custom_watch_list_impl:
        public test_watch_list_impl,
        public mock::object<mock_custom_watch_list_impl>,
        virtual public custom_watch_list_impl {
public:
    typedef mock_custom_watch_list_impl mock_type;

    MOCK_DEFINE_METHOD_2(add_watch, void (const std::wstring &, const node_handler &))
    MOCK_DEFINE_METHOD_1(remove_watch, void (node_id))
    MOCK_DEFINE_METHOD_3(set_watch, void(node_id, const std::wstring &, const node_handler &))
    MOCK_DEFINE_METHOD_2(set_fmt_opts, void (const dbgfmt::format_options & fmt_opts, const tree_updated_handler &))
};


}


