// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file forward.hpp
/// Contains forward declarations of cli types.

#pragma once

#include <memory>


namespace cxxdbg::dbg::cli {


class app_processor;
class exec_processor;
class exec_command;
class command;
class function_command;
class command_group;
class process_processor;
class target_processor;


typedef std::shared_ptr<exec_command> exec_command_sp;
typedef std::shared_ptr<command> command_sp;
typedef std::shared_ptr<function_command> function_command_sp;


}


