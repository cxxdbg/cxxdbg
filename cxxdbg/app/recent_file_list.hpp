// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file recent_file_list.hpp
/// Contains definition of the recent_file_list class.

#pragma once

#include "cxxdbg/util/signals.hpp"
#include <ranges.hpp>
#include <filesystem>
#include <list>


namespace cxxdbg {


class settings_storage;


/// Stores list of recent files. Contains logic related to adding file and clearing history
class recent_file_list {
public:
    using path_t = std::filesystem::path;

    /// Constructs recent file list with specified reference to settings storage,
    /// name of list in settings, and max size of histroy
    recent_file_list(settings_storage & sett,
                     const std::string & sett_name,
                     size_t max_size = 10);

    /// Return range of all files in list
    auto & files() const {
        return files_;
    }

    /// Adds recent file into history. Adds new entry into file list or moves
    /// existing entry into beginning of the list. Removes old entry if size
    /// of new list is greater than max size.
    void add(const path_t & p);

    /// Removes all files from list
    void clear();

    /// The signal is emitted after list of recent files has changed
    CXXDBG_DEFINE_SIGNALX(changed, void())

private:        
    /// Saves list of recent files into settings
    void save() const;

    /// Loads list of recent files from settings
    void load();

    settings_storage & sett_;       ///< Reference to settings storage
    std::string sett_name_;         ///< Name of list in settings
    size_t max_size_;               ///< Max history size
    std::list<path_t> files_;       ///< List of flies
};


}


