// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file mock_target_processor.hpp
/// Contains definition of the mock_target_processor class.

#pragma once

#include "cxxdbg/cli/target_processor.hpp"
#include "cxxdbg/mock/mock.hpp"


namespace cxxdbg::dbg::cli::test {


/// Mock class for target_processor abstract class
class mock_target_processor:
        public target_processor,
        public mock::object<mock_target_processor> {
public:
    MOCK_DEFINE_METHOD_4(load_target, void (
                             const std::filesystem::path & file_name,
                             const std::filesystem::path & work_dir,
                             const std::vector<std::string> & cmd_args,
                             const completion_handler & handler))

    MOCK_DEFINE_METHOD_1(close_target, void(const completion_handler & handler))
};


}


