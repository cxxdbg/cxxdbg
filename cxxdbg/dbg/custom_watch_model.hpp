// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file custom_watch_model.hpp
/// Contains definition of the custom_watch_model class.

#pragma once

#include "watch_model.hpp"
#include "watch_list.hpp"


namespace cxxdbg::dbg {


class custom_watch_list;


/// Tree view model for custom watch list
class custom_watch_model:
        public watch_model,
        virtual public tree_view_model {

public:
    /// Constructor, makes watch list model for specified watch list
    custom_watch_model(custom_watch_list & wlist);

    /// Destructor, destroys object
    virtual ~custom_watch_model();

    /// Returns true if cell is editable
    bool editable(const row_index & r, std::size_t c) override;

    /// Sets text for specified cell
    void set_text(const row_index & r, std::size_t c, const std::wstring & s) override;

    /// Adds new watch
    void add(const std::wstring & expr);

    /// Removes watch with specified index
    void remove(std::size_t index);

private:
    custom_watch_list & watch_;             ///< Reference to watch list
};


}


