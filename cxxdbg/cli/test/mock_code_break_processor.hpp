// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file mock_break_processor.hpp
/// Contains definition of the mock_code_break_processor class.

#pragma once

#include "cxxdbg/cli/code_break_processor.hpp"
#include "cxxdbg/mock/mock.hpp"


namespace cxxdbg::dbg::cli::test {


/// Mock class for the abstract break_processor class.
class mock_code_break_processor:
        public code_break_processor,
        public mock::object<mock_code_break_processor> {
public:

    MOCK_DEFINE_METHOD_2(delete_breakpoint, void(const breakpoint_id & id,
                                                 const async::result_handler<> & res))
    MOCK_DEFINE_METHOD_2(enable_breakpoint, void(const breakpoint_id & id,
                                                 const async::result_handler<> & res))
    MOCK_DEFINE_METHOD_2(disable_breakpoint, void(const breakpoint_id & id,
                                                  const async::result_handler<> & res))
    MOCK_DEFINE_METHOD_3(add_function_breakpoint, void(const std::string & name,
                                                       const break_properties & props,
                                                       const code_break_handler & handl))
    MOCK_DEFINE_METHOD_4(add_srcpos_breakpoint, void(const std::filesystem::path & file,
                                                     unsigned int line,
                                                     const break_properties & props,
                                                     const code_break_handler & handl))
    MOCK_DEFINE_METHOD_3(add_address_breakpoint, void(unsigned long addr,
                                                      const break_properties & props,
                                                      const code_break_handler & handl))
    MOCK_DEFINE_METHOD_3(set_breakpoint_props, void(const breakpoint_id & id,
                                                    const break_properties & props,
                                                    const async::result_handler<> & res))
    MOCK_DEFINE_METHOD_1(list_breakpoints, std::string(breakpoint_type))

    MOCK_DEFINE_METHOD_2(add_exception_thrown_breakpoint, void (const break_properties & props, const code_break_handler & handl))
    MOCK_DEFINE_METHOD_2(add_exception_caught_breakpoint, void (const break_properties & props, const code_break_handler & handl))
};


}


