// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file source_tree.cpp
/// Contains implementation of source_tree class and related classes.


#include "source_tree.hpp"
#include "source_tree_builder.hpp"
#include <ranges.hpp>
#include <memory>
#include <iostream>


namespace fs = std::filesystem;


namespace cxxdbg::dbg {


source_tree_item::source_tree_item(const std::filesystem::path & p, bool is_d):
path_{p}, is_dir_{is_d} {
    assert(path_.is_absolute() && "source tree item path should be absolute");
}


std::wstring source_tree_item::name() const {
    assert(path_.is_absolute() && "source tree item path should be absolute");
    auto nm = path_.filename();
    if (!nm.empty()) {
        return nm.wstring();
    }

    // if file name is empty then this must be a root
    return path_.root_path().wstring();
}


/// Merges with source tree builder
void source_tree::merge(const source_tree_builder_directory & tree) {
    merge_childs(nullptr, tree, {});
}


void source_tree::update_sorted_sources(std::vector<const source_file *> && sources) {
    sorted_sources_model_.set_sources(nullptr);
    sorted_sources_.clear();
    sorted_sources_ = std::move(sources);

    sorted_sources_model_.set_sources(&sorted_sources_);
}


const ro_tree_view_model & source_tree::sorted_sources() {
    return sorted_sources_model_;
}


const source_tree_item * source_tree::path_item(const fs::path & p) const {
    return child_with_path(nullptr, p);
}


const source_tree_item * source_tree::child_with_path(const source_tree_item * parent, const fs::path & p) const {

    // getting first item of path
    assert(!p.empty() && "path should not be empty");
    auto p_it = p.begin();
    assert(p_it != p.end() && "invalid path");
    auto first_item = *p_it;

    if (p.is_absolute()) {
#ifdef _WIN32
        // On Windows, root name (e.g. "C:") and root directory ("\") are separate items.
        // Merge them here into single subdirectory in root.
        ++p_it;
        assert(p_it != p.end() && "no root directory item in absolute path on Windows");
#endif

        // root_path returns combined root name and root directory (e.g. "C:\") on Windows
        // and / on POSIX systems
        first_item = p.root_path();
    }

    // constructing child path
    fs::path child_path;
    {
        ++p_it;
        for(auto end = p.end(); p_it != end; ++p_it) {
            child_path /= *p_it;
        }
    }

    // looking for child with name equal to first path component
    auto chlds = childs(parent);
    auto it = std::ranges::find_if(chlds, [first_item](auto && item) {
        return first_item == item->name();
    });
    if (it == std::ranges::end(chlds)) {
        return nullptr;
    }

    if (child_path.empty()) {
        return *it;
    }

    return child_with_path(*it, child_path);
}


/// Creates tree item from source tree builder directory
static std::unique_ptr<source_tree_item> make_directory_item(const source_tree_builder_directory & dir,
                                                             const fs::path & path) {
    auto res = std::make_unique<source_tree_item>(path, true);

    // adding subdirectories
    auto dirs_fn = [path](auto && dir) { return make_directory_item(*dir, path / dir->name()); };
    res->insert(res->childs_size(), dir.subdirs() | std::ranges::views::transform(dirs_fn));

    // adding files
    auto files_fn = [](auto && src_file) { return std::make_unique<source_tree_item>(src_file->path(), false); };
    res->insert(res->childs_size(), dir.files() | std::ranges::views::transform(files_fn));

    return res;
}


void source_tree::merge_childs(const source_tree_item * item,
                               const source_tree_builder_directory & dir,
                               const fs::path & curr_path) {

    size_t idx = 0;     // current position in tree model to insert/merge items

    // merging directories
    for (auto it = dir.subdirs_begin(), end = dir.subdirs_end(); it != end; ++it) {
        // looking for position to insert or merge existing directory
        while (true) {
            if (idx == childs_size(item)) {
                // end of items reached
                break;
            }

            auto ch = child(item, idx);
            if (!ch->is_directory()) {
                // files section reached
                break;
            }

            if (ch->name() >= it->name()) {
                break;
            }

            ++idx;
        }

        fs::path p = curr_path;
        if (p.empty()) {
            p = it->name();
        } else {
            p /= it->name();
        }

        if (idx != childs_size(item) && it->name() == child(item, idx)->name()) {
            // merging existing directory
            merge_childs(child(item, idx), *it, p);
        } else {
            // inserting new directory
            insert_node(item, idx, make_directory_item(*it, p));
        }
    
        ++idx;
    }


    // moving current index to first file
    while (idx != childs_size(item) && child(item, idx)->is_directory()) {
        ++idx;
    }


    // merging files
    for (auto it = dir.files_begin(), end = dir.files_end(); it != end; ++it) {
        auto fname = (*it)->path().filename().wstring();

        // looking for position to insert file
        while (true) {
            if (idx == childs_size(item)) {
                // end of items reached
                break;
            }

            if (child(item, idx)->name() >= fname) {
                break;
            }

            ++idx;
        }

        if (idx != childs_size(item) && fname == child(item, idx)->name()) {
            // skipping existing file
        } else {
            // inserting new file
            insert_node(item, idx, std::make_unique<source_tree_item>((*it)->path(), false));
        }

        ++idx;
    }
}


}
