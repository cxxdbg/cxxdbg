// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file module.hpp
/// Contains definition of module class.

#pragma once

#include "compile_unit.hpp"
#include "symbol.hpp"
#include <lldb/API/SBModule.h>
#include <ranges.hpp>
#include <filesystem>


namespace cxxdbg::dbg::core {


class symbol;


/// \class module
/// Represents single module in debugging info
class module {
public:
    /// Type of iterator over compiler units
    class const_compile_unit_iterator;

    /// Constructor, makes module with specified pointer to lldb module
    module(const lldb::SBModule & m);

    /// Copy constructor
    module(const module & m);

    /// Destructor, destroys object
    ~module();

    /// Returns path to module
    std::filesystem::path path() const;

    /// Returns number of compile units in module
    std::size_t compile_units_size() const;

    /// Returns compile unit at specified index
    compile_unit compile_unit_at(std::size_t i) const;

    /// Returns range of compile units in module
    auto compile_units() const {
        auto indexes = std::ranges::views::iota(size_t(), compile_units_size());
        auto fn = [this](size_t idx) { return compile_unit_at(idx); };
        return indexes | std::ranges::views::transform(fn);
    }

    /// Returns number of symbols in module
    size_t symbols_size() const;

    /// Returns symbol at specified index
    symbol symbol_at(size_t i) const;

    /// Returns range containing all symbols in module
    auto symbols() const {
        auto indexes = std::ranges::views::iota(size_t(), symbols_size());
        auto fn = [this](size_t idx) { return symbol_at(idx); };
        return indexes | std::ranges::views::transform(fn);
    }

    /// Reindexes symbols/types for module
    void reindex();

private:
    lldb::SBModule mod_;        ///< Pointer to lldb module
};


}
