// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file format.cpp
/// Contains implementations of value formatting functions.

#include "format.hpp"
#include "target.hpp"
#include "variable.hpp"
#include "dbgfmt/fixed_fmt_result.hpp"
#include "cxxdbg/log/log.hpp"


namespace cxxdbg::dbg::core {


dbgfmt::named_fmt_result format_value(target & targ,
                                   const std::string & name,
                                   const std::shared_ptr<dbgfmt::backend::value_impl> & val,
                                   const source_position & core_def_pos) {

    std::wstring wname{name.begin(), name.end()};
    
    try {
        dbgfmt::backend::source_position def_pos;
        if (core_def_pos) {
            def_pos.set_file(core_def_pos.path());
            def_pos.set_line(core_def_pos.line());
        }

        CXXDBG_LOG_DEBUG(fmt) << "begin format: " << name;
        auto fval = targ.format_value(val);
        CXXDBG_LOG_DEBUG(fmt) << "end format: " << name;

        return dbgfmt::named_fmt_result{wname, def_pos, fval};
    }
    catch (std::exception & err) {
        std::wostringstream msg;
        msg << "error: " << err.what();
        auto fval = dbgfmt::make_fmt_result<dbgfmt::fixed_fmt_result>(msg.str(),
                                                                msg.str(),
                                                                dbgfmt::backend::source_position{},
                                                                dbgfmt::backend::source_position{});
        return dbgfmt::named_fmt_result{wname, {}, fval};
    }
}


dbgfmt::named_fmt_result format_variable(target & targ, stack_frame & frame, const variable * var) {
    return format_value(targ, var->name(), frame.read_var_value(var), var->def_pos());
}


}