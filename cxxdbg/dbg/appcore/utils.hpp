// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file utils.hpp
/// Contains declaration of utility functions for appcore library

#pragma once

#include "cxxdbg/dbg/code_position_info.hpp"
#include "cxxdbg/dbg/core/code_position.hpp"


namespace cxxdbg::dbg::appcore {



/// Converts core source position to source position info
dbg::source_position_info extract_source_position_info(const core::source_position & pos);


/// Converts core code position to code position info
dbg::code_position_info extract_code_position_info(const core::target_base & targ,
                                                   const core::code_position & pos,
                                                   const core::code_position & call_pos);


}


