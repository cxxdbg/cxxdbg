// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file registers.cpp
/// Contains implementation of classes for reading list of registers.

#include "registers.hpp"
#include "target.hpp"
#include "cxxdbg/dbg/core/format.hpp"
#include "dbgfmt/temp_value_impl.hpp"
#include "cxxdbg/util/convert.hpp"


namespace cxxdbg::dbg::core {


/// Formatting result for registers group
class registers_group_fmt_result: public dbgfmt::fmt_result {
public:
    /// Constructs formatting result with specified reference to target and 
    /// registers group from core module
    registers_group_fmt_result(core::target & targ, const core::registers_group & grp):
        targ_{targ}, grp_{grp} {}

    /// Returns value of the value field
    std::wstring val() const override {
        return util::convert::to_wstring(grp_.name());
    }

    /// Returns value of type field
    std::wstring type() const override {
        return {};;
    }

    /// Returns source position of definition of object that is contained in address
    /// specied by value
    dbgfmt::backend::source_position val_pos() const override {
        return {};
    }

    /// Returns source position of definition of value type
    dbgfmt::backend::source_position type_pos() const override {
        return {};
    }

    /// Returns number of child nodes
    size_t childs_size() const override {
        return grp_.registers_size();
    }

    /// Returns child value at specified index
    dbgfmt::named_fmt_result child_at(size_t idx) const override {
        auto reg_val = grp_.register_value_at(idx);
        return format_value(targ_, grp_.register_name_at(idx), reg_val, {});
    }

private:
    core::target & targ_;           ///< Reference to target
    core::registers_group grp_;     ///< Registers group
};


std::shared_ptr<dbgfmt::backend::value_impl> registers_group::register_value_at(size_t idx) const {
    auto storage = register_value_storage_at(idx);
    auto type = register_value_type_at(idx);
    return std::make_shared<dbgfmt::temp_value_impl>(targ(),
                                                  type,
                                                  storage,
                                                  0,
                                                  targ().memory(),
                                                  targ().endianness());
}


registers_list::registers_list(target & targ, const lldb::SBValueList & vals):
registers_list_base{targ, vals}, targ_{targ} {
}


registers_group registers_list::group_at(size_t idx) const {
    return registers_group{group_base_at(idx)};
}


dbgfmt::named_fmt_result registers_list::group_fmt_result_at(size_t idx) const {
    auto grp = group_at(idx);
    auto grp_name = util::convert::to_wstring(grp.name());
    return dbgfmt::named_fmt_result{grp_name, {}, dbgfmt::make_fmt_result<registers_group_fmt_result>(targ_, grp)};
}


}
