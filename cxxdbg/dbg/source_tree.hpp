// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file source_tree.hpp
/// Contains definition of source_tree class and related classes.

#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <boost/iterator/iterator_adaptor.hpp>

#include <cxxdbg/util/signals.hpp>

#include "source_file_vector_model.hpp"
#include "cxxdbg/app/basic_tree_model.hpp"


namespace cxxdbg::dbg {


class source_directory_subdirs_iterator;
class source_directory_const_subdirs_iterator;

class source_file;
class source_tree_builder_directory;
class source_tree;
class source_tree_builder;
class source_tree_directory;


///\class source_tree_item
/// Base class for file and directory items
class source_tree_item: public basic_tree_model_node_base<source_tree_item> {
public:
    /// Constructs item with specified path and directory flag
    source_tree_item(const std::filesystem::path & p, bool is_d);

    /// Returns true if item is directory
    bool is_directory() const { return is_dir_; }

    /// Returns true if item is file
    bool is_file() const { return !is_directory(); }

    /// Returns item name
    std::wstring name() const;

    /// Returns item path
    const std::filesystem::path & path() const { return path_; }

private:
    std::filesystem::path path_;        ///< Item path
    bool is_dir_;                       ///< Is item directory?
};


/// \class source_tree
/// Represents source tree loaded from executable file.
class source_tree: public basic_tree_model<source_tree_item> {
public:    
    /// Merges with source tree builder
    void merge(const source_tree_builder_directory & tree);

    /// Updates sorted sources vector
    void update_sorted_sources(std::vector<const source_file *> && sources);

    /// Returns reference to model containing sorted list of
    /// source file names
    const ro_tree_view_model & sorted_sources();

    /// Returns tree item by path or nullptr if item not found
    const source_tree_item * path_item(const std::filesystem::path & p) const;

private:
    /// Merges child tree recursively
    void merge_childs(const source_tree_item * item,
                      const source_tree_builder_directory & dir,
                      const std::filesystem::path & curr_path);

    /// Returns child item with specified path
    const source_tree_item * child_with_path(const source_tree_item * item, const std::filesystem::path & p) const;

    /// Vector of source files ordered by file name
    std::vector<const source_file*> sorted_sources_;

    /// Tree model containing sorted list of source file names
    source_file_vector_model sorted_sources_model_{&sorted_sources_};
};


BOOST_CONCEPT_ASSERT((TreeModel<source_tree>));


}
