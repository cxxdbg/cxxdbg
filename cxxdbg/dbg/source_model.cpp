// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file source_model.cpp
/// Contains implementation of source_model class.

#include "source_model.hpp"
#include "source_file.hpp"


namespace fs = std::filesystem;


namespace cxxdbg::dbg {


/// Removes dot and dot dor from path
static fs::path remove_dots(const fs::path & p) {
    fs::path res;

    for (auto it = p.begin(), end = p.end(); it != end; ++it) {
        // skip dot
        if (*it == ".")
            continue;

        // handle dot dot
        if (*it == "..") {
            res = res.parent_path();
            continue;
        }

        res /= *it;
    }

    return res;
}


/// Removes dots from path and makes path absolute
static fs::path normalize_path(const fs::path & p) {
    return remove_dots(fs::absolute(p));
}


source_model::source_model() {
}


source_file * source_model::source(const fs::path & path) {

    fs::path cpath = normalize_path(path);

    assert(cpath.is_absolute() && "file path must be absolute");

    // looking for source in source path
    auto & sf = sources_[cpath.string()];
    if (sf) {
        // already exists
        return sf.get();
    }

    // adding new file to map
    sf.reset(new source_file{cpath});

    return sf.get();
}


std::vector<const source_file *> source_model::sorted_sources() const {
    std::vector<const source_file *> result;
    result.reserve(sources_.size());

    for (auto & item : sources_) {
        result.push_back(item.second.get());
    }

    return result;
}


}
