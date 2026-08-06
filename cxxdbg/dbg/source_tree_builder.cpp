// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "source_tree_builder.hpp"
#include <memory>


namespace fs = std::filesystem;


namespace cxxdbg::dbg {


source_tree_builder_directory::source_tree_builder_directory(const std::wstring & nm) :
        name_(nm) {
}


source_tree_builder_directory::~source_tree_builder_directory() {
}


source_tree_builder_directory::subdirs_iterator source_tree_builder_directory::find_subdir(const std::wstring & nm) {
    return subdirs_iterator(subdirs_.find(nm));
}


source_tree_builder_directory::const_subdirs_iterator source_tree_builder_directory::find_subdir(const std::wstring & nm) const {
    return const_subdirs_iterator(subdirs_.find(nm));
}


source_tree_builder_directory::files_iterator source_tree_builder_directory::find_file(const std::wstring & nm) {
    return files_iterator(files_.find(nm));
}


source_tree_builder_directory::const_files_iterator source_tree_builder_directory::find_file(const std::wstring & nm) const {
    return const_files_iterator(files_.find(nm));
}


source_tree_builder_directory::const_subdirs_iterator source_tree_builder_directory::subdirs_begin() const {
    return const_subdirs_iterator(subdirs_.begin());
}


source_tree_builder_directory::const_subdirs_iterator source_tree_builder_directory::subdirs_end() const {
    return const_subdirs_iterator(subdirs_.end());
}


source_tree_builder_directory::const_files_iterator source_tree_builder_directory::files_begin() const {
    return const_files_iterator(files_.begin());
}


source_tree_builder_directory::const_files_iterator source_tree_builder_directory::files_end() const {
    return const_files_iterator(files_.end());
}


void source_tree_builder_directory::clear() {
    subdirs_.clear();
    files_.clear();
}


const std::wstring & source_tree_builder_directory::name() const {
    return name_;
}


source_tree_builder_directory::subdirs_iterator source_tree_builder_directory::add_subdir(const std::wstring & name) {
    // find subdirectory in subdirs map
    auto it = subdirs_.find(name);
    if (it != subdirs_.end()) {
        // subdirectory already exists
        return subdirs_iterator(it);
    }

    // make new subdirectory and insert it to subdirs map
    source_directory_ptr subdir(new source_tree_builder_directory(name));
    auto res = subdirs_.insert(std::make_pair(name, subdir));
    assert(res.second && "Subdirectory already exists");

    return subdirs_iterator(res.first);
}


source_tree_builder_directory::files_iterator source_tree_builder_directory::add_file(const std::wstring & name, source_file * file) {
    assert(find_file(name) == files_end() && "File already exists");
    auto res = files_.insert(std::make_pair(name, file));

    assert(res.second && "File with specified name already exists");
    return files_iterator(res.first);
}


bool source_tree_builder_directory::add_file(const fs::path & path, source_file * file) {

    auto it = path.begin();
    assert(it != path.end() && "Path is empty");
    std::wstring name = it->wstring();

    if (path.is_absolute()) {
        // adding file into root

#ifdef _WIN32
        // On Windows, root name (e.g. "C:") and root directory ("\") are separate items.
        // Merge them here into single subdirectory in root.
        ++it;
        assert(it != path.end() && "no root directory item in absolute path on Windows");
#endif

        // root_path returns combined root name and root directory (e.g. "C:\") on Windows
        // and / on POSIX systems
        name = path.root_path().wstring();

    } else {
        name = it->wstring();
    }

    auto next = it;
    ++next;

    if (next == path.end()) {
        // end of path reached, adding file

        files_iterator fit = find_file(name);
        if (fit != files_end())
            return false;

        add_file(name, file);
        return true;
    }

    // looking for subdirectory
    subdirs_iterator sit = find_subdir(name);
    if (sit == subdirs_end()) {
        // adding new subdirectory
        sit = add_subdir(name);
    }

    // adding file in subdirectory
    fs::path subdir_path = *next;
    ++next;
    for (auto e = path.end(); next != e; ++next) {
        subdir_path /= *next;
    }

    return sit->add_file(subdir_path, file);
}


size_t source_tree_builder_directory::subdirs_count() const {
    return subdirs_.size();
}


size_t source_tree_builder_directory::files_count() const {
    return files_.size();
}


/// Adds source file to source tree
void source_tree_builder::add(source_file * src) {

    // ignore directories
    if (fs::is_directory(src->path())) {
        return;
    }

    if (files_.find(src->path()) == std::end(files_)) {
        files_.insert(src->path());
    }

    tree_->add_file(src->path(), src);
}


source_tree_builder::source_tree_builder():
tree_(std::make_unique<source_tree_builder_directory>(L"/")) {
}


source_tree_builder_directory & source_tree_builder::tree() {
    return *tree_;
}


}
