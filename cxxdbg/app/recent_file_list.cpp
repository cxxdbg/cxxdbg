// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file recent_file_list.cpp
/// Contains implementation of the recent_file_list class.

#include "recent_file_list.hpp"
#include "cxxdbg/app/settings_storage.hpp"
#include <ranges.hpp>


namespace cxxdbg {


recent_file_list::recent_file_list(settings_storage & sett,
                                   const std::string & sett_name,
                                   size_t max_size):
sett_{sett},
sett_name_{sett_name},
max_size_{max_size} {
    load();
}


void recent_file_list::add(const path_t & p) {
    // removing existing file from list
    files_.remove(p);

    // inserting file into beginning of list
    files_.insert(files_.begin(), p);

    // removing old files if size of list is greater than max size
    if (files_.size() > max_size_) {
        assert(files_.size() - 1 == max_size_ && "invalid size of list");
        files_.pop_back();
    }

    save();
    changed()();
}


void recent_file_list::save() const {
    sett_.write(sett_name_, files_);
}


void recent_file_list::load() {
    files_ = sett_.read<std::list<path_t>>(sett_name_, {});
}


void recent_file_list::clear() {
    files_.clear();
    save();
    changed()();
}


}
