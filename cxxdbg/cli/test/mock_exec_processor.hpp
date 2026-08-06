// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file mock_exec_processor.hpp
/// Contains definition of the mock_exec_processor class.

#pragma once

#include "cxxdbg/cli/exec_processor.hpp"
#include "cxxdbg/mock/mock.hpp"


namespace cxxdbg::dbg::cli::test {


/// Mock object for exec_processor abstract class
class mock_exec_processor:
        public exec_processor,
        public mock::object<mock_exec_processor> {
public:
    MOCK_DEFINE_METHOD_2(exec_cmd, void(const std::string & cmd, const result_handler & handl))
};



}


