// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file source_model.hpp
/// Contains definition of source_model class.

#pragma once

#include "forward.hpp"
#include "source_file.hpp"
#include "source_position.hpp"
#include <filesystem>
#include <map>
#include <set>
#include <boost/signals2/signal.hpp>


namespace cxxdbg::dbg {


/// \class source_model
/// Contains information about program sources and related logic.
class source_model {
public:
    /// Constructor, makes new source model with specified reference to
    /// application object
    source_model();

    /// Gets existing or adds new source_file object with specified path to source model.
    source_file * source(const std::filesystem::path & p);

    /// Returns sorted sources vector
    std::vector<const source_file *> sorted_sources() const;

private:
    /// Type of shared pointer to source file
    typedef std::shared_ptr<source_file> source_file_ptr;

    /// Type of map from file path to source file
    typedef std::map<std::string, source_file_ptr> path_source_map;

    path_source_map sources_;               ///< Map of all sources
};


}


