// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file registers_base.cpp
/// Contains implementation of base classes for classes used to reading register.s

#include "registers_base.hpp"
#include "lldb_value_storage.hpp"
#include "target_base.hpp"


namespace cxxdbg::dbg::core {


registers_group_base::registers_group_base(target_base & targ, const lldb::SBValue & val):
targ_{targ}, val_{val} {
}


registers_group_base::registers_group_base(const registers_group_base & grp) = default;


registers_group_base::~registers_group_base() {
}


std::string registers_group_base::name() const {
    auto name_cstr = val().GetName();
    return name_cstr ? name_cstr : "";
}


size_t registers_group_base::registers_size() const {
    return val().GetNumChildren();
}


std::string registers_group_base::register_name_at(size_t idx) const {
    auto v = val().GetChildAtIndex(idx);
    auto name_cstr = v.GetName();
    return name_cstr ? name_cstr : "";
}


const type_impl * registers_group_base::register_value_type_at(size_t idx) const {
    auto v = val().GetChildAtIndex(idx);
    assert(v.IsValid() && "invalid lldb value");
    auto t = v.GetType();
    assert(t.IsValid() && "invalid lldb value type");
    return targ_.get_or_create_type(t);
}


std::shared_ptr<dbgfmt::data::storage> registers_group_base::register_value_storage_at(size_t idx) const {
    return std::make_shared<lldb_value_storage>(val().GetChildAtIndex(idx));
}


lldb::SBValue registers_group_base::val() const {
    return val_;
}



registers_list_base::registers_list_base(target_base & targ, const lldb::SBValueList & vals):
targ_{targ}, values_{vals} {
}


registers_list_base::registers_list_base(const registers_list_base & rlb) = default;


registers_list_base::~registers_list_base() {
}


size_t registers_list_base::groups_size() const {
    return values().GetSize();
}


registers_group_base registers_list_base::group_base_at(size_t idx) const {
    return registers_group_base{targ_, values().GetValueAtIndex(idx)};
}


lldb::SBValueList registers_list_base::values() const {
    return values_;
}


}
