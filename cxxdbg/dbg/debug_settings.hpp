// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/* 
 * File:   debug_settings.hpp
 * Author: extremer
 *
 * Created on 17 Июль 2016 г., 21:33
 */

#pragma once

#include "cxxdbg/app/settings_storage.hpp"
#include "cxxdbg/dbg/core/regex_lists.hpp"
#include "dbgfmt/format_options.hpp"


namespace cxxdbg::dbg {


/// \class debug_settings
/// Represents debug settings

class debug_settings
{
public:
    /// constructor
    debug_settings(bool avoid_nodebug = false);
    
    ///< returns avoid_nodebug property value
    bool is_avoid_nodebug() const;
    
    ///< sets avoid_nodebug property value
    void set_avoid_nodebug(bool avoid_nodebug);
    
    ///< regexp lists for skipping
    const core::regex_lists & regexp_skip_lists() const;
    
    ///< Sets regexp lists for skipping
    void set_regexp_skip_lists(const core::regex_lists& value);
    
    ///< regex lists for stepping into
    const core::regex_lists & step_through_lists() const;
    
    ///< sets regexp lists for stepping into
    void set_step_through_lists(const core::regex_lists & value);

    /// regex lists matching functions which should be grouped in call stack model
    const core::regex_lists & group_functions_lists() const;

    /// sets regex lists matching functions which should be grouped in call stack model
    void set_group_functions_lists(const core::regex_lists & value);

    /// Returns const reference to format options
    auto & fmt_opts() const { return fmt_opts_; }

    /// Returns reference to format options
    auto & fmt_opts() { return fmt_opts_; }

    /// Sets format options
    void set_fmt_opts(dbgfmt::format_options fopts) { fmt_opts_ = std::move(fopts); }
    
    ///< saves debug settings to settings storage
    void save_settings(settings_storage& storage);
    
    /// loads debug setting from settings storage
    void load_settings(settings_storage& storage);
    
    ///< not equality operator
    bool operator != (const debug_settings & other) const;
    
    ///< equality operator
    bool operator == (const debug_settings & other) const;
    
private:
    bool avoid_nodebug_;
    core::regex_lists regex_skip_lists_;
    core::regex_lists regex_step_through_lists_;
    core::regex_lists regex_group_functions_lists_;
    dbgfmt::format_options fmt_opts_;
};


}


