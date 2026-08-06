// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file app_processor.hpp
/// Contains definition of the app_processor class.

#pragma once

#include "code_break_processor.hpp"
#include "exec_processor.hpp"
#include "frame_processor.hpp"
#include "platform_processor.hpp"
#include "process_processor.hpp"
#include "target_processor.hpp"
#include "thread_processor.hpp"
#include "watch_processor.hpp"


namespace cxxdbg::dbg::cli {


/// Abstract CLI processor which is used by app_command_interpreter
class app_processor:
        public code_break_processor,
        public exec_processor,
        public frame_processor,
        public platform_processor,
        public process_processor,
        public target_processor,
        public thread_processor,
        public watch_processor
{
};


}


