// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file stack_frame.hpp
/// Contains definition of the stack_frame class.

#pragma once

#include "registers.hpp"
#include "stack_frame_base.hpp"


namespace cxxdbg::dbg::core {

class target;
class thread;


/// Represents stack frame in target being debugged.
class stack_frame: public stack_frame_base {
public:
    /// Constructs stack frame from thread adn index of stack frame in thread
    stack_frame(const thread & thrd, size_t idx);

    /// Reads value of frame varialbe
    std::shared_ptr<dbgfmt::backend::value_impl> read_var_value(const variable * var);

    /// Evaluates expression in stack frame
    std::shared_ptr<dbgfmt::backend::value_impl> eval_expr(const std::string & expr) const;

    /// Reads registers list
    registers_list read_registers() const;

private:
    target & targ_;         ///< Reference to target this stack frame belongs to
};


}
