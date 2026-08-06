// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file stack_frame.cpp
/// Contains implementation of the stack_frame calss.

#include "stack_frame.hpp"
#include "target.hpp"
#include "thread.hpp"
#include "dbgfmt/simple_value_impl.hpp"
#include "dbgfmt/temp_value_impl.hpp"


namespace cxxdbg::dbg::core {


stack_frame::stack_frame(const thread & thrd, size_t idx):
stack_frame_base{thrd, idx},
targ_{thrd.targ()} {
}


std::shared_ptr<dbgfmt::backend::value_impl> stack_frame::read_var_value(const variable * var) {
    // reading variable type
    auto type = read_var_type(var);

    // trying read value address
    auto addr = read_var_addr(var);
    if (addr != UINT64_MAX) {
        // creating value with memory storage and address read
        return std::make_shared<dbgfmt::simple_value_impl>(targ_,
                                                        type,
                                                        targ_.memory(),
                                                        targ_.endianness(),
                                                        addr,
                                                        dbgfmt::backend::source_position{});
    }

    // creating storage for lldb value
    auto val_st = get_var_lldb_value_storage(var);

    // creating temporary value that takes ownership of storage
    return std::make_shared<dbgfmt::temp_value_impl>(targ_,
                                                  type,
                                                  val_st,
                                                  0,
                                                  targ_.memory(),
                                                  targ_.endianness());
}


std::shared_ptr<dbgfmt::backend::value_impl> stack_frame::eval_expr(const std::string & expr) const {
    auto [storage, type] = do_eval_expr(expr);
    return std::make_shared<dbgfmt::temp_value_impl>(targ_,
                                                  type,
                                                  storage,
                                                  0,
                                                  targ_.memory(),
                                                  targ_.endianness());
}


registers_list stack_frame::read_registers() const {
    return registers_list{targ_, read_registers_base()};
}


}
