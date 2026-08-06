// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file source_file.hpp
/// Contains definition of source_file class.

#pragma once

#include <filesystem>
#include <list>
#include <map>


namespace cxxdbg::dbg {


/// \class source_file
/// Represents single source file in executable being debugged
class source_file {
public:
    /// Constructor, makes source file with specified reference to application
    /// object and path
    explicit source_file(const std::filesystem::path & p);

    /// Returns file path
    const std::filesystem::path & path() const;

    std::wstring name() const;

private:
    std::filesystem::path path_;          ///< File path
};


}


