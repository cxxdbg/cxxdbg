// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "debug_settings.hpp"
#include "regex_lists_settings.hpp"
#include "core/regex_lists_defaults.hpp"


namespace cxxdbg::dbg {


debug_settings::debug_settings(bool avoid_nodebug):
    avoid_nodebug_(avoid_nodebug) {
}


bool debug_settings::is_avoid_nodebug() const
{
    return avoid_nodebug_;
}


void debug_settings::set_avoid_nodebug(bool avoid_nodebug)
{
    avoid_nodebug_ = avoid_nodebug;
}


const core::regex_lists& debug_settings::regexp_skip_lists() const 
{ 
    return regex_skip_lists_; 
}


void debug_settings::set_regexp_skip_lists(const core::regex_lists & value)
{ 
    regex_skip_lists_ = value;
}

const core::regex_lists & debug_settings::step_through_lists() const
{
    return regex_step_through_lists_;
}


void debug_settings::set_step_through_lists(const core::regex_lists & value)
{
    regex_step_through_lists_ = value;
}


void debug_settings::save_settings(settings_storage & storage)
{
    storage.write<bool>("debug/avoid_nodebug", avoid_nodebug_);
    
    save_regex_lists_to_settings(storage, core::functions_to_skip::default_storage_name(), regex_skip_lists_);
    save_regex_lists_to_settings(storage, core::functions_to_step_into::default_storage_name(), regex_step_through_lists_);
    save_regex_lists_to_settings(storage, core::functions_to_group::default_storage_name(), regex_group_functions_lists_);

    storage.write("debug/fmt/show_raw_data", fmt_opts_.show_raw_data());
    storage.write("debug/fmt/hex", fmt_opts_.hex());
    storage.write("debug/fmt/min_hex_size", fmt_opts_.min_hex_size());
    storage.write("debug/fmt/show_ptr_addr", fmt_opts_.show_ptr_addr());
    storage.write("debug/fmt/show_rec_addr", fmt_opts_.show_rec_addr());
}


void debug_settings::load_settings(settings_storage & storage)
{
    avoid_nodebug_ = storage.read("debug/avoid_nodebug", false);
    
    regex_skip_lists_ = load_regex_lists_from_settings(storage,
                                                       core::functions_to_skip::default_storage_name(),
                                                       core::functions_to_skip::default_value());
    regex_step_through_lists_ = load_regex_lists_from_settings(storage,
                                                               core::functions_to_step_into::default_storage_name(),
                                                               core::functions_to_step_into::default_value());
    regex_group_functions_lists_ = load_regex_lists_from_settings(storage,
                                                                  core::functions_to_group::default_storage_name(),
                                                                  core::functions_to_group::default_value());

    fmt_opts_.set_show_raw_data(storage.read<bool>("debug/fmt/show_raw_data", fmt_opts_.show_raw_data()));
    fmt_opts_.set_hex(storage.read<bool>("debug/fmt/hex", fmt_opts_.hex()));
    fmt_opts_.set_min_hex_size(storage.read<size_t>("debug/fmt/min_hex_size", fmt_opts_.min_hex_size()));
    fmt_opts_.set_show_ptr_addr(storage.read<bool>("debug/fmt/show_ptr_addr", fmt_opts_.show_ptr_addr()));
    fmt_opts_.set_show_rec_addr(storage.read<bool>("debug/fmt/show_rec_addr", fmt_opts_.show_rec_addr()));
}


bool debug_settings::operator != (const debug_settings & other) const
{
    return !(*this == other);
}


bool debug_settings::operator == (const debug_settings & other) const
{
    return avoid_nodebug_ == other.avoid_nodebug_ && 
            regex_skip_lists_ == other.regex_skip_lists_ && 
            regex_step_through_lists_ == other.regex_step_through_lists_ &&
            regex_group_functions_lists_ == other.regex_group_functions_lists_ &&
            fmt_opts_ == other.fmt_opts_;
}


void debug_settings::set_group_functions_lists(const core::regex_lists & value) {
    regex_group_functions_lists_ = value;
}


const core::regex_lists & debug_settings::group_functions_lists() const {
    return regex_group_functions_lists_;
}


}
