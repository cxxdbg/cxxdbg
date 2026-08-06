// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file break_group.hpp
/// Contains definition of the break_group class.

#pragma once

#include "break_base_group.hpp"
#include "code_break_processor.hpp"


namespace cxxdbg::dbg::cli {


class code_break_processor;
class exec_processor;


/// Breakpoints command group
class break_group: public break_base_group {
public:
    /// Breakpoint type tag
    using tag = bp_tag;

    /// Constructor, makes group with specified references to exec processor
    /// and breakpoint processor
    break_group(exec_processor & exec_proc, code_break_processor & break_proc);

private:
    void reg_set(code_break_processor & proc);
};


}


