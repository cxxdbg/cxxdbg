// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file compile_unit.hpp
/// Contains definition of compile_unit class

#pragma once

#include <lldb/API/SBCompileUnit.h>
#include <filesystem>
#include <boost/iterator/iterator_adaptor.hpp>


namespace cxxdbg::dbg::core {


/// \class compile_unit
/// Represents single compile unit in debug info
class compile_unit {
public:
    /// Type of terator over support files
    class const_support_file_iterator;

    /// Constructor, makes compile unit with specified pointer to lldb
    /// compile unit
    compile_unit(const lldb::SBCompileUnit cu);

    /// Destructor, destroys object
    ~compile_unit();

    /// Returns path to compile unit source
    std::filesystem::path source_path() const;

    /// Returns number of support files
    std::size_t support_files_size() const;

    /// Returns path to support file at specified index
    std::filesystem::path support_file_at(std::size_t i) const;

    /// Returns iterator pointing to the first support file
    const_support_file_iterator support_files_begin() const;

    /// Returns iterator pointing to the one past the last support file
    const_support_file_iterator support_files_end() const;

private:
    lldb::SBCompileUnit cunit_;         ///< Pointer to lldb compile unit
};


class compile_unit::const_support_file_iterator: public boost::iterator_adaptor <
    const_support_file_iterator,
    std::size_t,
    std::filesystem::path,
    boost::random_access_traversal_tag,
    std::filesystem::path,
    std::size_t
> {
public:
    /// Constructor, makes iterator with speicified reference to compile
    /// unit and support file index
    const_support_file_iterator(const compile_unit & cu, std::size_t i);

    /// Dereferences iterator
    reference dereference() const;

private:
    const compile_unit & cu_;       ///< Reference to compile unit
};


}
