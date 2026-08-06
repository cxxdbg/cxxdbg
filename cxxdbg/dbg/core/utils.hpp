// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file utils.hpp
/// Contains declaration of small utility functions

#pragma once

#include "source_position.hpp"
#include "dbgfmt/backend/source_position.hpp"
#include <lldb/API/SBDefines.h>
#include <filesystem>


namespace cxxdbg::dbg::core {


class code_model;


/// Converts lldb SBFileSpec to path
std::filesystem::path lldb_sb_file_spec_to_path(const lldb::SBFileSpec sp);

/// Converts lldb FileSpec to path
std::filesystem::path lldb_file_spec_to_path(const lldb_private::FileSpec & sp);

/// Returns function name contained at lldb address
std::string func_name_at_address(const code_model & cm, lldb::SBAddress addr);


}
