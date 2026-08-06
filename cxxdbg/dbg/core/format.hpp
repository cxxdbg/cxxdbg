// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file format.hpp
/// Contains declarations of value formatting functions.

#pragma once

#include "source_position.hpp"
#include "stack_frame.hpp"
#include "dbgfmt/backend/value_impl.hpp"
#include "dbgfmt/fmt_result.hpp"
#include <memory>


namespace cxxdbg::dbg::core {

class target;


/// Formats value
dbgfmt::named_fmt_result format_value(target & targ,
                                   const std::string & name,
                                   const std::shared_ptr<dbgfmt::backend::value_impl> & val,
                                   const source_position & core_def_pos);


/// Reads and formats variable value for specified frame
dbgfmt::named_fmt_result format_variable(target & targ,
                                      stack_frame & frame,
                                      const variable * var);


}
