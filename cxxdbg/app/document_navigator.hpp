// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file document_navigator.hpp
/// Contains definition of the document_navigator class.

#pragma once

#include <filesystem>


namespace cxxdbg {


/// Abstract document navigator that can navigate to positions in text files
class document_navigator {
public:
    /// Default virtual destructor
    virtual ~document_navigator() = default;

    /// Shows specified position
    virtual void show_pos(const std::filesystem::path & path, size_t line) = 0;
};


}
