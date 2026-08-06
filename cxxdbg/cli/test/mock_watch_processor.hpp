// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file mock_watch_processor.hpp
/// Contains definition of the mock_watch_processor class.

#pragma once

#include "../watch_processor.hpp"
#include "cxxdbg/mock/mock.hpp"


namespace cxxdbg::dbg::cli::test {


/// Mock class for the abstract watch_processor class.
class mock_watch_processor:
        public watch_processor,
        public mock::object<mock_watch_processor> {
public:

    MOCK_DEFINE_METHOD_2(delete_breakpoint, void(const id_t & id,
                                                 const async::result_handler<> & res))
    MOCK_DEFINE_METHOD_2(enable_breakpoint, void(const id_t & id,
                                                 const async::result_handler<> & res))
    MOCK_DEFINE_METHOD_2(disable_breakpoint, void(const id_t & id,
                                                  const async::result_handler<> & res))
    MOCK_DEFINE_METHOD_3(set_breakpoint_props, void(const id_t & id,
                                                    const break_properties & props,
                                                    const async::result_handler<> & res))
    MOCK_DEFINE_METHOD_1(list_breakpoints, std::string(breakpoint_type))

    MOCK_DEFINE_METHOD_3(add_var_watchpoint, void(const std::string & name,
                                                  const watch_create_properties & props,
                                                  const watch_handler & handl))
    MOCK_DEFINE_METHOD_3(add_expr_watchpoint, void(const std::string & expr,
                                                   const watch_create_properties & props,
                                                   const watch_handler & handl))
};


}


