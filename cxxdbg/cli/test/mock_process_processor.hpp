// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file mock_process_processor.hpp
/// Contains definition of the mock_process_processor class.

#pragma once

#include "cxxdbg/cli/process_processor.hpp"
#include "cxxdbg/mock/mock.hpp"


namespace cxxdbg::dbg::cli::test {


/// Mock class for process_processor abstract class
class mock_process_processor:
        public process_processor,
        public mock::object<mock_process_processor> {
public:
    MOCK_DEFINE_METHOD_2(attach_pid, void (unsigned long pid, const process_handler & handler))
    MOCK_DEFINE_METHOD_2(attach_name, void (const std::string &, const process_handler & handler))
    MOCK_DEFINE_METHOD_1(attach, void (const process_handler & handler))
    MOCK_DEFINE_METHOD_1(resume, void (const process_handler & handler))
    MOCK_DEFINE_METHOD_1(detach, void (const process_handler & handler))
    MOCK_DEFINE_METHOD_1(interrupt, void (const process_handler & handler))
    MOCK_DEFINE_METHOD_1(kill, void (const process_handler & handler))
    MOCK_DEFINE_METHOD_4(launch, void (const std::filesystem::path & work_dir,
                                       const std::vector<std::string> & cmd_args,
                                       bool save_default,
                                       const process_handler & handler))
};


}


