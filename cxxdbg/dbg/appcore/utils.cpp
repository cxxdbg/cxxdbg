// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file utils.cpp
/// Contains implementations of utility functions for appcore library

#include "utils.hpp"
#include "cxxdbg/dbg/core/target.hpp"
#include "cxxdbg/dbg/core/variable.hpp"
#include "dbgfmt/fixed_fmt_result.hpp"


namespace cxxdbg::dbg::appcore {


dbg::source_position_info extract_source_position_info(const core::source_position & pos) {
    return pos.is_valid() ?
           dbg::source_position_info(pos.path(), pos.line()) :
           dbg::source_position_info();
}



dbg::code_position_info extract_code_position_info(const core::target_base & targ,
                                                   const core::code_position & pos,
                                                   const core::code_position & call_pos) {
    dbg::source_position_info src_pos = extract_source_position_info(call_pos.get_src_pos());
    return dbg::code_position_info(pos.load_addr(),
                                   pos.file_addr(),
                                   call_pos.get_func_name(),
                                   src_pos);
}


}
