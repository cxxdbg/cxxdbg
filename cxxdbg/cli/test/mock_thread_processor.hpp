// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file mock_thread_processor.hpp
/// Contains definition of the mock_thread_processor class.

#pragma once

#include "cxxdbg/cli/thread_processor.hpp"
#include "cxxdbg/mock/mock.hpp"


namespace cxxdbg::dbg::cli::test {


/// Mock class for the abstract thread_processor class.
class mock_thread_processor:
        public thread_processor,
        public mock::object<mock_thread_processor>
{
public:
    MOCK_DEFINE_METHOD_2(select_thread, void (std::size_t index, const async::result_handler<> & h))
    MOCK_DEFINE_METHOD_5(step_into, void (tribool,
                                          const std::string &,
                                          const std::string &,
                                          const std::string &,
                                          const async::result_handler<> & h))
    MOCK_DEFINE_METHOD_2(step_over, void (const std::string &, const async::result_handler<> & h))
    MOCK_DEFINE_METHOD_3(step_out, void(tribool, const std::string &, const async::result_handler<> & h))
    MOCK_DEFINE_METHOD_1(inst_step_into, void(const async::result_handler<> & h))
    MOCK_DEFINE_METHOD_1(inst_step_over, void(const async::result_handler<> & h))
    MOCK_DEFINE_METHOD_2(step_until, void(unsigned int line, const async::result_handler<> & h))
};


}


