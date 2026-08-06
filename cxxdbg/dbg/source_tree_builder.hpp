// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/*
 * File:   source_tree_builder.hpp
 * Author: yzarudny
 *
 * Created on October 5, 2016, 9:59 AM
 */

#pragma once

#include <boost/iterator/iterator_adaptor.hpp>

#include <ranges.hpp>
#include <filesystem>
#include <map>
#include <set>
#include <memory>
#include <string>
#include <vector>

#include "source_file.hpp"
#include "source_file_vector_model.hpp"


namespace cxxdbg::dbg {


class source_directory_subdirs_iterator;
class source_directory_const_subdirs_iterator;


/// \class source_directory
/// Represents single source directory used in source_tree_builder
class source_tree_builder_directory {

    /// Type of shared pointer to source directory
    typedef std::shared_ptr<source_tree_builder_directory> source_directory_ptr;

    /// Type of name -> source_directory map
    typedef std::map<std::wstring, source_directory_ptr> source_directory_map;

    /// Type of name -> source_file map
    typedef std::map<std::wstring, source_file *> source_file_map;

public:
    /// Type of iterator over subdirectories
    typedef source_directory_subdirs_iterator subdirs_iterator;

    /// Type of const iterator over subdirectories
    typedef source_directory_const_subdirs_iterator const_subdirs_iterator;


    /// Type of iterator over files
    class files_iterator : public boost::iterator_adaptor<
            files_iterator,
            source_file_map::iterator,
            source_file *,
            boost::use_default,
            source_file *
    > {
    public:
        typedef source_file base_element_type;

        explicit files_iterator(const source_file_map::iterator & it) :
                iterator_adaptor_(it) {}

        reference dereference() const {
            return base()->second;
        }
    };


    /// Type of const iterator over files
    class const_files_iterator : public boost::iterator_adaptor<
            const_files_iterator,
            source_file_map::const_iterator,
            const source_file *,
            boost::use_default,
            const source_file *
    > {
    public:
        typedef source_file base_element_type;

        explicit const_files_iterator(const source_file_map::const_iterator & it) :
                iterator_adaptor_(it) {}

        const_files_iterator(const files_iterator & it) :
                iterator_adaptor_(it.base()) {}

        reference dereference() const {
            return base()->second;
        }
    };


    /// Constructor, makes empty source directory with specified name
    source_tree_builder_directory(const std::wstring & nm = std::wstring());

    /// Destructor, destroys directory
    ~source_tree_builder_directory();

    /// Returns iterator pointing to subdirectory with specified name or
    /// iterator pointing to the one past last subdirectory if not exists
    subdirs_iterator find_subdir(const std::wstring & nm);

    /// Returns const iterator poiting to subdirectory with specified name or
    /// iterator pointing to the one past last subdirectory if not exists
    const_subdirs_iterator find_subdir(const std::wstring & nm) const;

    /// Returns iterator pointing to file with specified name,
    /// or iterator pointing to the one past last file if not exists
    files_iterator find_file(const std::wstring & nm);

    /// Returns const iterator pointing to file with specified name,
    /// or iterator pointing to the one past last file if not exists
    const_files_iterator find_file(const std::wstring & nm) const;

    /// Returns subdirs count
    size_t subdirs_count() const;

    /// Returns files count
    size_t files_count() const;

    /// Returns iterator pointing to the first subdirectory
    const_subdirs_iterator subdirs_begin() const;

    /// Returns iterator pointing to the one past last subdirectory
    const_subdirs_iterator subdirs_end() const;

    /// Returns range of subdirectories
    auto subdirs() const {
        auto fn = [](auto && entry) { return entry.second.get(); };
        return subdirs_ | std::ranges::views::transform(fn);
    }

    /// Returns iterator pointing to the first file
    const_files_iterator files_begin() const;

    /// Returns iterator pointing to the one past last file
    const_files_iterator files_end() const;

    /// Returns range of files
    auto files() const {
        auto fn = [](auto && entry) { return const_cast<const source_file*>(entry.second); };
        return files_ | std::ranges::views::transform(fn);
    }

    /// Removes all files and subdirs from directory
    void clear();

    /// Returns directory name
    const std::wstring & name() const;

    /// Adds subdirectory with specified name. Returns
    /// iterator pointing to added directory or iterator
    /// pointing to existing directory.
    subdirs_iterator add_subdir(const std::wstring & name);

    /// Adds file to source directory. Returns iterator pointint to
    /// added file. File with same name should not exist
    files_iterator add_file(const std::wstring & name, source_file * file);

    /// Adds source file with specified path to subdirectory, recursively
    /// adding subdirectories. Returns true if file was added, or false,
    /// if file with same path already exists.
    bool add_file(const std::filesystem::path & path, source_file * file);

private:
    std::wstring name_;                     ///< Directory name
    source_directory_map subdirs_;          ///< List of subdirectories
    source_file_map files_;                 ///< List of files
};


/// \class source_directory_subdirs_iterator
/// Type of iterator over subdirectories in source directory
class source_directory_subdirs_iterator : public boost::iterator_adaptor<
        source_directory_subdirs_iterator,
        std::map<std::wstring, std::shared_ptr<source_tree_builder_directory>>::iterator,
        source_tree_builder_directory
> {
public:
    typedef source_tree_builder_directory base_element_type;

    explicit source_directory_subdirs_iterator(const base_type & it) :
            iterator_adaptor_(it) {
    }

    reference dereference() const {
        return *base()->second;
    }
};

/// Type of const iterator over subdirectories
class source_directory_const_subdirs_iterator : public boost::iterator_adaptor<
        source_directory_const_subdirs_iterator,
        std::map<std::wstring, std::shared_ptr<source_tree_builder_directory>>::const_iterator,
        const source_tree_builder_directory
> {
public:
    typedef source_tree_builder_directory base_element_type;

    explicit source_directory_const_subdirs_iterator(const base_type & it) :
            iterator_adaptor_(it) {}

    source_directory_const_subdirs_iterator(const source_directory_subdirs_iterator & it) :
            iterator_adaptor_(it.base()) {}

    reference dereference() const {
        return *base()->second;
    }
};


class source_tree_builder {
public:
    /// Default constructor
    source_tree_builder();

    ~source_tree_builder() = default;

    /// Adds source file to source tree builder
    void add(source_file * src);

    /// User sources
    source_tree_builder_directory & tree();

private:
    std::unique_ptr<source_tree_builder_directory> tree_;   ///< Tree of sources
    std::set<std::filesystem::path> files_;                 ///< set of file paths
};

}

