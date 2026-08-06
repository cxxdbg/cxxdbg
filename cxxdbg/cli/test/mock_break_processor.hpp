// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file mock_break_processor.hpp
/// Contains definition of the mock_break_processor class

#pragma once

#include "../break_processor.hpp"
#include "cxxdbg/mock/mock.hpp"


namespace cxxdbg::dbg::cli::test {


struct mock_bp_tag {};


/// Mock class for the abstract break_base_processor class.
class mock_break_processor:
        public break_processor,
        public mock::object<mock_break_processor> {
public:
    using bp_tag = mock_bp_tag;

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
};


}


